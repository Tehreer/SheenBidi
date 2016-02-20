/*
 * Copyright (C) 2016 Muhammad Tayyab Akram
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

extern "C" {
#include <Source/SBBracketType.h>
}

#include "Utilities/Math.h"
#include "Utilities/Converter.h"
#include "Utilities/ArrayBuilder.h"
#include "Utilities/FileBuilder.h"

#include "PairingLookupGenerator.h"

using namespace std;
using namespace SheenBidi::Parser;
using namespace SheenBidi::Generator;
using namespace SheenBidi::Generator::Utilities;

static const size_t MIN_SEGMENT_SIZE = 32;
static const size_t MAX_SEGMENT_SIZE = 512;

static const string DIFFERENCES_ARRAY_TYPE = "static const SBInt16";
static const string DIFFERENCES_ARRAY_NAME = "_SBPairDifferences";

static const string DATA_ARRAY_TYPE = "static const SBUInt8";
static const string DATA_ARRAY_NAME = "_SBPairData";

static const string INDEXES_ARRAY_TYPE = "static const SBUInt16";
static const string INDEXES_ARRAY_NAME = "_SBPairIndexes";

PairingLookupGenerator::DataSegment::DataSegment(size_t index, DataSet dataset)
    : index(index)
    , dataset(dataset)
{
}

const string PairingLookupGenerator::DataSegment::macroLine() const {
    return ("#define " + macroName() + " 0x" + Converter::toHex(index, 3));
}

const string PairingLookupGenerator::DataSegment::macroName() const {
    return (DATA_ARRAY_NAME + "_" + Converter::toHex(index, 3));
}

PairingLookupGenerator::PairingLookupGenerator(const BidiMirroring& bidiMirroring, const BidiBrackets& bidiBrackets)
    : m_bidiMirroring(bidiMirroring)
    , m_bidiBrackets(bidiBrackets)
    , m_firstCodePoint(0)
    , m_lastCodePoint(max(bidiMirroring.lastCodePoint(), bidiBrackets.lastCodePoint()))
    , m_segmentSize(0)
{
}

void PairingLookupGenerator::setSegmentSize(size_t segmentSize) {
    m_segmentSize = min(MAX_SEGMENT_SIZE, max(MIN_SEGMENT_SIZE, segmentSize));
}

void PairingLookupGenerator::analyzeData() {
    cout << "Analyzing data for pairing lookup." << endl;

    size_t memory = analyzeData(true);

    cout << "  Segment Size: " << m_segmentSize << endl;
    cout << "  Required Memory: " << memory << " bytes" << endl;

    cout << "Finished analysis." << endl << endl;
}

size_t PairingLookupGenerator::analyzeData(bool all) {
    if (all || !m_segmentSize) {
        size_t minMemory = SIZE_MAX;
        size_t segmentSize = 0;

        m_segmentSize = MIN_SEGMENT_SIZE;
        while (m_segmentSize <= MAX_SEGMENT_SIZE) {
            collectData();

            size_t memory = m_dataSize + (m_differencesSize * 2) + (m_indexesSize * 2);
            if (memory < minMemory) {
                segmentSize = m_segmentSize;
                minMemory = memory;
            }

            m_segmentSize++;
        }
        m_segmentSize = segmentSize;

        return minMemory;
    }

    return 0;
}

void PairingLookupGenerator::collectData() {
    size_t pairCount = m_lastCodePoint - m_firstCodePoint;
    size_t maxSegments = Math::FastCeil(pairCount, m_segmentSize);

    m_data.clear();
    m_data.reserve(maxSegments);

    m_differences.clear();
    m_differences.reserve(32);
    m_differences.push_back(0);

    m_indexes.clear();
    m_indexes.reserve(maxSegments);

    m_dataSize = 0;

    for (size_t i = 0; i < maxSegments; i++) {
        uint32_t segmentStart = (uint32_t)(m_firstCodePoint + (i * m_segmentSize));
        uint32_t segmentEnd = min(m_lastCodePoint, (uint32_t)(segmentStart + m_segmentSize - 1));
        
        DataSet dataset(new UnsafeDataSet());
        dataset->reserve(m_segmentSize);

        for (uint32_t codePoint = segmentStart; codePoint <= segmentEnd; codePoint++) {
            uint32_t mirror = m_bidiMirroring.mirrorForCodePoint(codePoint);
            uint32_t bracket = m_bidiBrackets.pairedBracketForCodePoint(codePoint);
            char bracketType = m_bidiBrackets.pairedBracketTypeForCodePoint(codePoint);
            if (bracket && mirror != bracket) {
                cout << "Logic Broken:" << endl
                     << "  Code Point: " << codePoint << endl
                     << "  Mirror: " << mirror << endl
                     << "  Paired Bracket: " << bracket << endl;
            }

            int16_t difference = (mirror ? mirror - codePoint : 0);

            auto begin = m_differences.begin();
            auto end = m_differences.end();
            auto match = find(begin, end, difference);

            uint8_t data;
            if (match != end) {
                data = distance(begin, match);
            } else {
                data = m_differences.size();
                m_differences.push_back(difference);
            }

            switch (bracketType) {
            case 'o':
                data |= SBBracketTypeOpen;
                break;

            case 'c':
                data |= SBBracketTypeClose;
                break;
            }

            dataset->push_back(data);
        }

        size_t segmentIndex = SIZE_MAX;
        size_t segmentCount = m_data.size();
        for (size_t j = 0; j < segmentCount; j++) {
            if (*m_data[j].dataset == *dataset) {
                segmentIndex = j;
            }
        }

        if (segmentIndex == SIZE_MAX) {
            segmentIndex = m_data.size();
            m_data.push_back(DataSegment(m_dataSize, dataset));
            m_dataSize += dataset->size();
        }

        m_indexes.push_back(&m_data.at(segmentIndex));
    }

    m_differencesSize = m_differences.size();
    m_indexesSize = m_indexes.size();
}

void PairingLookupGenerator::generateFile(const std::string &directory) {
    analyzeData(false);
    collectData();

    ArrayBuilder arrDifferences;
    arrDifferences.setDataType(DIFFERENCES_ARRAY_TYPE);
    arrDifferences.setName(DIFFERENCES_ARRAY_NAME);
    arrDifferences.setElementSpace(5);

    auto diffPtr = m_differences.begin();
    auto diffEnd = m_differences.end();
    for (; diffPtr != diffEnd; diffPtr++) {
        const string &element = Converter::toString(*diffPtr);
        bool isLast = (diffPtr == (diffEnd - 1));

        arrDifferences.appendElement(element);
        if (!isLast) {
            arrDifferences.newElement();
        }
    }

    ArrayBuilder arrData;
    arrData.setDataType(DATA_ARRAY_TYPE);
    arrData.setName(DATA_ARRAY_NAME);
    arrData.setElementSpace(3);

    auto dataPtr = m_data.begin();
    auto dataEnd = m_data.end();
    for (; dataPtr != dataEnd; dataPtr++) {
        const DataSegment &data = *dataPtr;
        bool isLast = (dataPtr == (dataEnd - 1));

        arrData.append(data.macroLine());
        arrData.newLine();

        size_t length = data.dataset->size();

        for (size_t j = 0; j < length; j++) {
            const string &element = Converter::toString(data.dataset->at(j));
            arrData.appendElement(element);
            arrData.newElement();
        }

        if (!isLast) {
            arrData.newLine();
        }
    }

    ArrayBuilder arrIndexes;
    arrIndexes.setDataType(INDEXES_ARRAY_TYPE);
    arrIndexes.setName(INDEXES_ARRAY_NAME);

    size_t segmentStart = m_firstCodePoint;
    auto indexPtr = m_indexes.begin();
    auto indexEnd = m_indexes.end();
    for (; indexPtr != indexEnd; indexPtr++) {
        const DataSegment &data = **indexPtr;
        bool isLast = (indexPtr == (indexEnd - 1));

        arrIndexes.appendElement(data.macroName());
        if (!isLast) {
            arrIndexes.newElement();
        }

        size_t segmentEnd = segmentStart + data.dataset->size() - 1;
        string description = "/**< %"
                           + Converter::toHex(segmentStart, 4) + ".."
                           + Converter::toHex(segmentEnd, 4)
                           + "*/";

        arrIndexes.appendTabs(4);
        arrIndexes.append(description);
        if (!isLast) {
            arrIndexes.newLine();
        }

        segmentStart += m_segmentSize;
    }

    TextBuilder mirror;
    string maxCodePoint = "0x" + Converter::toHex(m_lastCodePoint, 4);
    string segmentSize = "0x" + Converter::toHex(m_segmentSize, 3);
    mirror.append("").newLine();
    mirror.append("SB_INTERNAL SBCodepoint SBPairingDetermineMirror(SBCodepoint codepoint) {").newLine();
    mirror.appendTabs(1).append("if (codepoint <= " + maxCodePoint + ") {").newLine();
    mirror.appendTabs(2).append("SBInt16 diff = _SBPairDifferences[").newLine();
    mirror.appendTabs(2).append("                _SBPairData[").newLine();
    mirror.appendTabs(2).append("                 _SBPairIndexes[").newLine();
    mirror.appendTabs(2).append("                      codepoint / " + segmentSize).newLine();
    mirror.appendTabs(2).append("                 ] + (codepoint % " + segmentSize + ")").newLine();
    mirror.appendTabs(2).append("                ] & SBBracketTypeInverseMask").newLine();
    mirror.appendTabs(2).append("               ];").newLine();
    mirror.newLine();
    mirror.appendTabs(2).append("if (diff != 0) {").newLine();
    mirror.appendTabs(3).append("return (codepoint + diff);").newLine();
    mirror.appendTabs(2).append("}").newLine();
    mirror.appendTabs(1).append("}").newLine();
    mirror.newLine();
    mirror.appendTabs(1).append("return 0;").newLine();
    mirror.append("}").newLine();

    TextBuilder bracket;
    bracket.append("SB_INTERNAL SBCodepoint SBPairingDetermineBracketPair(SBCodepoint codepoint, SBBracketType *type) {").newLine();
    bracket.appendTabs(1).append("if (codepoint <= " + maxCodePoint + ") {").newLine();
    bracket.appendTabs(2).append("SBUInt8 data = _SBPairData[").newLine();
    bracket.appendTabs(2).append("                _SBPairIndexes[").newLine();
    bracket.appendTabs(2).append("                     codepoint / " + segmentSize).newLine();
    bracket.appendTabs(2).append("                ] + (codepoint % " + segmentSize + ")").newLine();
    bracket.appendTabs(2).append("               ];").newLine();
    bracket.appendTabs(2).append("*type = (data & SBBracketTypePrimaryMask);").newLine();
    bracket.newLine();
    bracket.appendTabs(2).append("if (*type != 0) {").newLine();
    bracket.appendTabs(3).append("SBInt16 diff = _SBPairDifferences[").newLine();
    bracket.appendTabs(3).append("                data & SBBracketTypeInverseMask").newLine();
    bracket.appendTabs(3).append("               ];").newLine();
    bracket.appendTabs(3).append("return (codepoint + diff);").newLine();
    bracket.appendTabs(2).append("}").newLine();
    bracket.appendTabs(1).append("} else {").newLine();
    bracket.appendTabs(2).append("*type = SBBracketTypeNone;").newLine();
    bracket.appendTabs(1).append("}").newLine();
    bracket.newLine();
    bracket.appendTabs(1).append("return 0;").newLine();
    bracket.append("}").newLine();

    FileBuilder header(directory + "/SBPairingLookup.h");
    header.append("/*").newLine();
    header.append(" * Automatically generated by SheenBidiGenerator tool.").newLine();
    header.append(" * DO NOT EDIT!!").newLine();
    header.append(" */").newLine();
    header.newLine();
    header.append("#ifndef _SB_INTERNAL_CHAR_TYPE_LOOKUP_H").newLine();
    header.append("#define _SB_INTERNAL_CHAR_TYPE_LOOKUP_H").newLine();
    header.newLine();
    header.append("#include <SBConfig.h>").newLine();
    header.append("#include <SBTypes.h>").newLine();
    header.newLine();
    header.append("#include \"SBBracketType.h\"").newLine();
    header.newLine();
    header.append("SB_INTERNAL SBCodepoint SBPairingDetermineMirror(SBCodepoint codepoint);").newLine();
    header.append("SB_INTERNAL SBCodepoint SBPairingDetermineBracketPair(SBCodepoint codepoint, SBBracketType *bracketType);").newLine();
    header.newLine();
    header.append("#endif").newLine();

    FileBuilder source(directory + "/SBPairingLookup.c");
    source.append("/*").newLine();
    source.append(" * Automatically generated by SheenBidiGenerator tool.").newLine();
    source.append(" * DO NOT EDIT!!").newLine();
    source.append(" */").newLine();
    source.newLine();
    source.append("#include <SBConfig.h>").newLine();
    source.append("#include <SBTypes.h>").newLine();
    source.newLine();
    source.append("#include \"SBBracketType.h\"").newLine();
    source.append("#include \"SBPairingLookup.h\"").newLine();
    source.newLine();
    source.append(arrDifferences);
    source.newLine();
    source.append(arrData);
    source.newLine();
    source.append(arrIndexes);
    source.append(mirror).newLine();
    source.append(bracket);
}
