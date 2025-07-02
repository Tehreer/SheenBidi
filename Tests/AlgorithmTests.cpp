/*
 * Copyright (C) 2015-2025 Muhammad Tayyab Akram
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

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <SheenBidi/SheenBidi.h>

#include <Parser/BidiCharacterTest.h>
#include <Parser/BidiMirroring.h>
#include <Parser/BidiTest.h>

#include "Utilities/Convert.h"

#include "Configuration.h"
#include "AlgorithmTests.h"

using namespace std;
using namespace SheenBidi;
using namespace SheenBidi::Parser;
using namespace SheenBidi::Utilities;

constexpr uint8_t LEVEL_X = UINT8_MAX;

struct TestInput {
    const SBCodepoint *codePointBuffer;
    SBUInteger codePointCount;
    SBLevel baseLevel;
    SBMirrorLocatorRef mirrorLocator;
};

struct TestOutput {
    uint8_t paragraphLevel;
    const vector<uint8_t> &levels;
    const vector<size_t> &order;
    const uint32_t *mirrors;
    size_t mirrorCount;
};

static bool testLevels(const TestInput &input, const TestOutput &output, SBLineRef line, SBLevel paragraphLevel);
static bool testOrder(const TestInput &input, const TestOutput &output, SBLineRef line);
static bool testMirrors(const TestInput &input, const TestOutput &output, SBLineRef line);

static SBUInteger loadCodePoints(const vector<uint32_t> &codePoints, SBCodepoint *codePointBuffer) {
    for (auto &cp : codePoints) {
        *(codePointBuffer++) = cp;
    }

    return codePoints.size();
}

static SBUInteger loadCodePoints(const vector<string> &bidiTypes, SBCodepoint *codePointBuffer) {
    for (auto &t : bidiTypes) {
        *(codePointBuffer++) = Convert::toCodePoint(t);
    }

    return bidiTypes.size();
}

static size_t loadMirrors(const BidiMirroring &bidiMirroring,
    const SBCodepoint *codePointBuffer, SBUInteger codePointCount,
    const vector<uint8_t> &levels, uint32_t *mirrorBuffer)
{
    size_t mirrorCount = 0;

    for (size_t i = 0; i < codePointCount; i++) {
        auto level = levels.at(i);
        if (level & 1) {
            mirrorBuffer[i] = bidiMirroring.mirrorOf(codePointBuffer[i]);
            if (mirrorBuffer[i]) {
                mirrorCount += 1;
            }
        } else {
            mirrorBuffer[i] = 0;
        }
    }

    return mirrorCount;
}

static bool testAlgorithm(const TestInput &input, const TestOutput &output) {
    bool passed = true;

    SBCodepointSequence sequence;
    sequence.stringEncoding = SBStringEncodingUTF32;
    sequence.stringBuffer = input.codePointBuffer;
    sequence.stringLength = input.codePointCount;

    auto algorithm = SBAlgorithmCreate(&sequence);
    auto paragraph = SBAlgorithmCreateParagraph(algorithm, 0, input.codePointCount, input.baseLevel);
    auto paragraphlevel = SBParagraphGetBaseLevel(paragraph);

    if (paragraphlevel == output.paragraphLevel || output.paragraphLevel == LEVEL_X) {
        auto line = SBParagraphCreateLine(paragraph, 0, input.codePointCount);

        passed &= testLevels(input, output, line, paragraphlevel);
        passed &= testOrder(input, output, line);
        passed &= testMirrors(input, output, line);

        SBLineRelease(line);
    } else {
        if (Configuration::DISPLAY_ERROR_DETAILS) {
            cout << "Test failed due to paragraph level mismatch." << endl;
            cout << "  Discovered Paragraph Level: " << (int)paragraphlevel << endl;
            cout << "  Expected Paragraph Level: " << (int)output.paragraphLevel << endl;
        }

        passed &= false;
    }

    SBParagraphRelease(paragraph);
    SBAlgorithmRelease(algorithm);

    return passed;
}

static bool testLevels(const TestInput &input, const TestOutput &output, SBLineRef line, SBLevel paragraphLevel) {
    auto runArray = SBLineGetRunsPtr(line);
    auto runCount = SBLineGetRunCount(line);

    for (size_t i = 0; i < runCount; i++) {
        auto runPtr = &runArray[i];
        auto start = runPtr->offset;
        auto end = start + runPtr->length - 1;
        auto level = runPtr->level;

        if (end >= input.codePointCount) {
            if (Configuration::DISPLAY_ERROR_DETAILS) {
                cout << "Test failed due to invalid run indexes." << endl;
                cout << "  Text Length: " << input.codePointCount << endl;
                cout << "  Run Start Index: " << start << endl;
                cout << "  Run End Index: " << end << endl;
            }
            return false;
        }

        auto oldLevel = paragraphLevel;
        for (size_t j = start; j--;) {
            if (output.levels.at(j) != LEVEL_X) {
                oldLevel = output.levels.at(j);
                break;
            }
        }

        for (size_t j = start; j <= end; j++) {
            if (output.levels.at(j) == LEVEL_X) {
                if (level != oldLevel) {
                    cout << "Warning: Level X should be equal to previous level." << endl;
                }
                continue;
            }

            if (output.levels.at(j) != level) {
                if (Configuration::DISPLAY_ERROR_DETAILS) {
                    cout << "Test failed due to level mismatch." << endl;
                    cout << "  Text Index: " << j << endl;
                    cout << "  Discovered Level: " << (int)level << endl;
                    cout << "  Expected Level: " << (int)output.levels.at(j) << endl;
                }
                return false;
            }

            oldLevel = output.levels.at(j);
        }
    }

    return true;
}

bool testOrder(const TestInput &input, const TestOutput &output, SBLineRef line) {
    bool passed = true;

    auto runArray = SBLineGetRunsPtr(line);
    auto runCount = SBLineGetRunCount(line);

    size_t lgcIndex = 0;    // Logical Index (incremented from zero to char count)
    size_t ordIndex = 0;    // Order Index (not incremented when level x is found)
    size_t dcvIndex = 0;    // Discovered Visual Index
    size_t expIndex = 0;    // Expected Visual Index

    for (size_t i = 0; i < runCount; i++) {
        auto runPtr = &runArray[i];
        auto start = runPtr->offset;
        auto end = start + runPtr->length - 1;
        auto level = runPtr->level;

        if (level & 1) {
            dcvIndex = end;

            for (size_t j = start; j <= end; j++, dcvIndex--) {
                lgcIndex++;

                if (output.levels.at(dcvIndex) == LEVEL_X) {
                    continue;
                }

                expIndex = output.order.at(ordIndex++);
                if (expIndex != dcvIndex) {
                    passed = false;
                    break;
                }
            }
        } else {
            for (dcvIndex = start; dcvIndex <= end; dcvIndex++) {
                lgcIndex++;

                if (output.levels.at(dcvIndex) == LEVEL_X) {
                    continue;
                }

                expIndex = output.order.at(ordIndex++);
                if (expIndex != dcvIndex) {
                    passed = false;
                    break;
                }
            }
        }
    }

    if (!passed) {
        if (Configuration::DISPLAY_ERROR_DETAILS) {
            cout << "Test failed due to invalid visual order." << endl;
            cout << "  Logical Index: " << lgcIndex << endl;
            cout << "  Discovered Visual Index: " << dcvIndex << endl;
            cout << "  Expected Visual Index: " << expIndex << endl;
        }
    }

    return passed;
}

bool testMirrors(const TestInput &input, const TestOutput &output, SBLineRef line) {
    SBMirrorLocatorLoadLine(input.mirrorLocator, line, input.codePointBuffer);

    auto agent = SBMirrorLocatorGetAgent(input.mirrorLocator);
    SBMirrorLocatorReset(input.mirrorLocator);

    size_t locatedMirrors = 0;

    while (SBMirrorLocatorMoveNext(input.mirrorLocator)) {
        if (agent->index < input.codePointCount) {
            if (agent->mirror == output.mirrors[agent->index]) {
                locatedMirrors += 1;
            } else {
                if (Configuration::DISPLAY_ERROR_DETAILS) {
                    cout << "Test failed due to mirror mismatch." << endl;
                    cout << "  Text Index: " << agent->index << endl;
                    cout << "  Discovered Mirror: "
                         << uppercase << hex << setfill('0')
                         << setw(4) << agent->mirror << endl
                         << nouppercase << dec << setfill('\0');
                    cout << "  Expected Mirror: "
                         << uppercase << hex << setfill('0')
                         << setw(4) << output.mirrors[agent->index] << endl
                         << nouppercase << dec << setfill('\0');
                }

                return false;
            }
        } else {
            if (Configuration::DISPLAY_ERROR_DETAILS) {
                cout << "Test failed due to invalid mirror index." << endl;
                cout << "  Text Length: " << input.codePointCount;
                cout << "  Located Index: " << agent->index << endl;
            }

            return false;
        }
    }

    if (locatedMirrors != output.mirrorCount) {
        if (Configuration::DISPLAY_ERROR_DETAILS) {
            cout << "Test failed due to mismatch in mirror count." << endl;
            cout << "  Discovered Mirrors: " << locatedMirrors << endl;
            cout << "  Expected Mirrors: " << output.mirrorCount << endl;
        }

        return false;
    }

    return true;
}

static void displayTestCase(size_t testCounter, const BidiTest::TestCase &testCase) {
    if (Configuration::DISPLAY_TEST_CASE) {
        cout << "Bidi Test # " << testCounter << ':' << endl;

        cout << "Paragraph Directions: " << endl << '\t';
        if (testCase.directions & BidiTest::ParagraphDirection::Auto) {
            cout << "Auto ";
        }
        if (testCase.directions & BidiTest::ParagraphDirection::LTR) {
            cout << "LTR ";
        }
        if (testCase.directions & BidiTest::ParagraphDirection::RTL) {
            cout << "RTL ";
        }
        cout << endl;

        cout << "Character Types: " << endl << '\t';
        for (auto &type : testCase.types) {
            cout << type << '\t';
        }
        cout << endl;

        cout << "Levels: " << endl << '\t';
        for (auto level : testCase.levels) {
            if (level == LEVEL_X) {
                cout << 'x';
            } else {
                cout << (int)level;
            }
            cout << '\t';
        }
        cout << endl;

        cout << "Reordered Indexes: " << endl << '\t';
        if (testCase.order.empty()) {
            cout << "Empty";
        } else {
            for (auto index : testCase.order) {
                cout << index << '\t';
            }
        }
        cout << endl;
    }
}

static void displayTestCase(size_t testCounter, const BidiCharacterTest::TestCase &testCase) {
    if (Configuration::DISPLAY_TEST_CASE) {
        cout << "Bidi Character Test # " << testCounter << endl;

        cout << "Paragraph Direction: " << endl << '\t';
        switch (testCase.paragraphDirection) {
        case BidiCharacterTest::ParagraphDirection::LTR:
            cout << "LTR" << endl;
            break;

        case BidiCharacterTest::ParagraphDirection::RTL:
            cout << "RTL" << endl;
            break;

        default:
            cout << "Auto" << endl;
            break;
        }

        cout << "Characters: " << endl << '\t';
        cout << uppercase << hex << setfill('0');
        for (auto codePoint : testCase.text) {
            cout << setw(4) << codePoint << '\t';
        }
        cout << nouppercase << dec << setfill('\0');
        cout << endl;

        cout << "Paragraph Level: " << endl << '\t';
        cout << (int)testCase.paragraphLevel << '\t';
        cout << endl;

        cout << "Levels: " << endl << '\t';
        for (auto level : testCase.levels) {
            if (level == LEVEL_X) {
                cout << 'x';
            } else {
                cout << (int)level;
            }
            cout << '\t';
        }
        cout << endl;

        cout << "Reordered Indexes: " << endl << '\t';
        if (testCase.order.empty()) {
            cout << "Empty";
        } else {
            for (auto index : testCase.order) {
                cout << index << '\t';
            }
        }
        cout << endl;
    }
}

static size_t testAllCases(BidiTest &bidiTest, const BidiMirroring &bidiMirroring) {
    auto mirrorLocator = SBMirrorLocatorCreate();

    SBCodepoint codePointBuffer[256] = { 0 };
    SBCodepoint mirrorBuffer[256] = { 0 };

    size_t testCounter = 0;
    size_t failures = 0;

    auto &testCase = bidiTest.testCase();
    bidiTest.reset();

    for (; bidiTest.fetchNext(); testCounter++) {
        displayTestCase(testCounter, testCase);

        bool passed = true;
        auto codePointCount = loadCodePoints(testCase.types, codePointBuffer);
        auto mirrorCount = loadMirrors(bidiMirroring, codePointBuffer, codePointCount,
            testCase.levels, mirrorBuffer);

        if (testCase.directions & BidiTest::ParagraphDirection::Auto) {
            passed &= testAlgorithm(
                { codePointBuffer, codePointCount, SBLevelDefaultLTR, mirrorLocator },
                { LEVEL_X, testCase.levels, testCase.order, mirrorBuffer, mirrorCount });
        }

        if (testCase.directions & BidiTest::ParagraphDirection::LTR) {
            passed &= testAlgorithm(
                { codePointBuffer, codePointCount, 0, mirrorLocator },
                { LEVEL_X, testCase.levels, testCase.order, mirrorBuffer, mirrorCount });
        }

        if (testCase.directions & BidiTest::ParagraphDirection::RTL) {
            passed &= testAlgorithm(
                { codePointBuffer, codePointCount, 1, mirrorLocator },
                { LEVEL_X, testCase.levels, testCase.order, mirrorBuffer, mirrorCount });
        }

        if (!passed) {
            failures += 1;
        }
    }

    SBMirrorLocatorRelease(mirrorLocator);

    return failures;
}

static size_t testAllCases(BidiCharacterTest &bidiCharacterTest, const BidiMirroring &bidiMirroring) {
    auto mirrorLocator = SBMirrorLocatorCreate();

    SBCodepoint codePointBuffer[256] = { 0 };
    SBCodepoint mirrorBuffer[256] = { 0 };

    size_t testCounter = 0;
    size_t failures = 0;

    auto &testCase = bidiCharacterTest.testCase();
    bidiCharacterTest.reset();

    for (; bidiCharacterTest.fetchNext(); testCounter++) {
        displayTestCase(testCounter, testCase);

        auto codePointCount = loadCodePoints(testCase.text, codePointBuffer);
        auto mirrorCount = loadMirrors(bidiMirroring, codePointBuffer, codePointCount,
            testCase.levels, mirrorBuffer);
        SBLevel inputLevel;

        switch (testCase.paragraphDirection) {
        case BidiCharacterTest::ParagraphDirection::LTR:
            inputLevel = 0;
            break;

        case BidiCharacterTest::ParagraphDirection::RTL:
            inputLevel = 1;
            break;

        default:
            inputLevel = SBLevelDefaultLTR;
            break;
        }

        if (!testAlgorithm(
                { codePointBuffer, codePointCount, inputLevel, mirrorLocator },
                { testCase.paragraphLevel, testCase.levels, testCase.order,
                  mirrorBuffer, mirrorCount })) {
            failures += 1;
        }
    }

    SBMirrorLocatorRelease(mirrorLocator);

    return failures;
}

AlgorithmTests::AlgorithmTests(BidiTest &bidiTest, BidiCharacterTest &bidiCharacterTest,
    const BidiMirroring &bidiMirroring) :
    m_bidiTest(bidiTest),
    m_bidiCharacterTest(bidiCharacterTest),
    m_bidiMirroring(bidiMirroring)
{
}

void AlgorithmTests::run()
{
    testAlgorithm();
    testMulticharNewline();
}

void AlgorithmTests::testAlgorithm() {
    cout << "Running algorithm tests." << endl;

    size_t failures = 0;
    failures += testAllCases(m_bidiTest, m_bidiMirroring);
    failures += testAllCases(m_bidiCharacterTest, m_bidiMirroring);

    cout << failures << " error/s." << endl;
    cout << endl;

    assert(failures == 0);
}

void AlgorithmTests::testMulticharNewline() {
    cout << "Running multi character new line test." << endl;

    size_t failed = 0;
    SBCodepoint codepointArray[] = { 'L', 'i', 'n', 'e', '\r', '\n', '.' };
    SBUInteger codepointCount = sizeof(codepointArray) / sizeof(SBCodepoint);

    SBCodepointSequence sequence;
    sequence.stringEncoding = SBStringEncodingUTF32;
    sequence.stringBuffer = codepointArray;
    sequence.stringLength = codepointCount;

    auto algorithm = SBAlgorithmCreate(&sequence);
    auto paragraph = SBAlgorithmCreateParagraph(algorithm, 0, codepointCount, 0);
    auto offset = SBParagraphGetOffset(paragraph);
    auto length = SBParagraphGetLength(paragraph);
    auto levels = SBParagraphGetLevelsPtr(paragraph);

    if (offset != 0 && length != 6) {
        failed = 1;

        if (Configuration::DISPLAY_ERROR_DETAILS) {
            cout << "Test failed due to invalid paragraph range." << endl;
            cout << "  Paragraph Offset: " << offset << endl;
            cout << "  Paragraph Length: " << length << endl;
            cout << "  Expected Offset: " << 0 << endl;
            cout << "  Expected Length: " << 6 << endl;
        }
    }

    for (size_t i = 0; i < length; i++) {
        if (levels[i] != 0) {
            failed = 1;

            if (Configuration::DISPLAY_ERROR_DETAILS) {
                cout << "Test failed due to level mismatch." << endl;
                cout << "  Text Index: " << i << endl;
                cout << "  Discovered Level: " << (int)levels[i] << endl;
                cout << "  Expected Level: " << 0 << endl;
            }
        }
    }

    SBParagraphRelease(paragraph);
    SBAlgorithmRelease(algorithm);

    cout << failed << " error/s." << endl;
    cout << endl;

    assert(failed == false);
}

#ifdef STANDALONE_TESTING

int main(int argc, const char *argv[]) {
    const char *dir = argv[1];

    BidiMirroring bidiMirroring(dir);
    BidiTest bidiTest(dir);
    BidiCharacterTest bidiCharacterTest(dir);

    AlgorithmTests algorithmTests(bidiTest, bidiCharacterTest, bidiMirroring);
    algorithmTests.run();

    return 0;
}

#endif
