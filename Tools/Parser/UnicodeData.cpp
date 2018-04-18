/*
 * Copyright (C) 2018 Muhammad Tayyab Akram
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
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "UnicodeData.h"

using namespace std;
using namespace SheenBidi::Parser;

static const string FILE_UNICODE_DATA = "UnicodeData.txt";

static inline void getField(const string &data, size_t offset, int field, string &result) {
    result.clear();

    if (offset != SIZE_MAX) {
        size_t start = offset;
        size_t end = offset;

        for (int i = 0; i < field; i++) {
            start = end + 1;
            end = data.find(';', start);
        }

        result.append(data.c_str() + start, end - start);
    }
}

UnicodeData::UnicodeData(const string &directory) :
    m_offsets(0x110000, SIZE_MAX),
    m_lastCodePoint(0)
{
    ifstream fileStream(directory + "/" + FILE_UNICODE_DATA, ios::binary);
    char buffer[4096];

    while (fileStream.read(buffer, sizeof(buffer))) {
        m_data.append(buffer, sizeof(buffer));
    }
    m_data.append(buffer, (size_t)fileStream.gcount());

    istringstream dataStream(m_data, ios::binary);
    uint32_t codePoint;

    while (dataStream >> hex >> setw(6) >> codePoint) {
        streamoff offset = dataStream.tellg();
        m_offsets[codePoint] = (size_t)offset;

        if (codePoint > m_lastCodePoint) {
            m_lastCodePoint = codePoint;
        }

        dataStream.ignore(1024, '\n');
    }
}

uint32_t UnicodeData::firstCodePoint() const {
    return 0;
}

uint32_t UnicodeData::lastCodePoint() const {
    return m_lastCodePoint;
}

size_t UnicodeData::offset(uint32_t codePoint) const {
    if (codePoint <= m_lastCodePoint) {
        return m_offsets[codePoint];
    }

    return SIZE_MAX;
}

void UnicodeData::getCharacterName(uint32_t codePoint, string &characterName) const {
    getField(m_data, offset(codePoint), 1, characterName);
}

void UnicodeData::getGeneralCategory(uint32_t codePoint, string &generalCategory) const {
    getField(m_data, offset(codePoint), 2, generalCategory);
}

void UnicodeData::getCombiningClass(uint32_t codePoint, string &combiningClass) const {
    getField(m_data, offset(codePoint), 3, combiningClass);
}

void UnicodeData::getBidirectionalCategory(uint32_t codePoint, string &bidirectionalCategory) const {
    getField(m_data, offset(codePoint), 4, bidirectionalCategory);
}

void UnicodeData::getDecompositionMapping(uint32_t codePoint, string &decompositionMapping) const {
    getField(m_data, offset(codePoint), 5, decompositionMapping);
}

void UnicodeData::getDecimalDigitValue(uint32_t codePoint, string &decimalDigitValue) const {
    getField(m_data, offset(codePoint), 6, decimalDigitValue);
}

void UnicodeData::getDigitValue(uint32_t codePoint, string &digitValue) const {
    getField(m_data, offset(codePoint), 7, digitValue);
}

void UnicodeData::getNumericValue(uint32_t codePoint, string &numericValue) const {
    getField(m_data, offset(codePoint), 8, numericValue);
}

void UnicodeData::getMirrored(uint32_t codePoint, string &mirrored) const {
    getField(m_data, offset(codePoint), 9, mirrored);
}

void UnicodeData::getOldName(uint32_t codePoint, string &oldName) const {
    getField(m_data, offset(codePoint), 10, oldName);
}

void UnicodeData::getCommentField(uint32_t codePoint, string &commentField) const {
    getField(m_data, offset(codePoint), 11, commentField);
}

void UnicodeData::getUppercaseMapping(uint32_t codePoint, string &uppercaseMapping) const {
    getField(m_data, offset(codePoint), 12, uppercaseMapping);
}

void UnicodeData::getLowercaseMapping(uint32_t codePoint, string &lowercaseMapping) const {
    getField(m_data, offset(codePoint), 13, lowercaseMapping);
}

void UnicodeData::getTitlecaseMapping(uint32_t codePoint, string &titlecaseMapping) const {
    getField(m_data, offset(codePoint), 14, titlecaseMapping);
}
