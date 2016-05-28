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

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

extern "C" {
#include <SheenBidi.h>
}

#include <Parser/BidiTest.h>
#include <Parser/BidiBrackets.h>
#include <Parser/BidiMirroring.h>

#include "Utilities/Converter.h"

#include "Configuration.h"
#include "AlgorithmTester.h"

using namespace std;
using namespace SheenBidi::Parser;
using namespace SheenBidi::Tester;
using namespace SheenBidi::Tester::Utilities;

static uint8_t LEVEL_X = UINT8_MAX;

AlgorithmTester::AlgorithmTester(BidiTest *bidiTest, BidiCharacterTest *bidiCharacterTest, BidiMirroring *bidiMirroring)
    : m_bidiTest(bidiTest)
    , m_bidiCharacterTest(bidiCharacterTest)
    , m_bidiMirroring(bidiMirroring)
    , m_codepointSequence(SBCodepointSequenceCreate())
    , m_runAdapter(SBRunAdapterCreate())
    , m_mirrorLocator(SBMirrorLocatorCreate())
{
}

AlgorithmTester::~AlgorithmTester() {
    SBCodepointSequenceRelease(m_codepointSequence);
    SBRunAdapterRelease(m_runAdapter);
    SBMirrorLocatorRelease(m_mirrorLocator);
}

void AlgorithmTester::test() {
    if (m_bidiTest || m_bidiCharacterTest) {
        cout << "Running algorithm tester." << endl;

        testCounter = 0;
        failCounter = 0;

        if (m_bidiTest) {
            analyzeBidiTest();
        }

        if (m_bidiCharacterTest) {
            analyzeBidiCharacterTest();
        }

        cout << failCounter << " error/s." << endl;
    } else {
        cout << "No test case found for algorithm tester." << endl;
    }

    cout << endl;
}

void AlgorithmTester::loadCharacters(const vector<string> &types) {
    SBCodepoint *chars = m_genChars;

    for (auto &t : types) {
        *(chars++) = Converter::toCodePoint(t);
    }

    m_charCount = types.size();
}

void AlgorithmTester::loadCharacters(const vector<uint32_t> &codePoints) {
    SBCodepoint *chars = m_genChars;

    for (auto c : codePoints) {
        *(chars++) = c;
    }

    m_charCount = codePoints.size();
}

void AlgorithmTester::loadMirrors() {
    if (m_bidiMirroring) {
        m_mirrorCount = 0;

        for (size_t i = 0; i < m_charCount; i++) {
            uint8_t level = m_levels->at(i);
            if (level & 1) {
                m_genMirrors[i] = m_bidiMirroring->mirrorForCodePoint(m_genChars[i]);
                if (m_genMirrors[i]) {
                    m_mirrorCount++;
                }
            } else {
                m_genMirrors[i] = 0;
            }
        }
    }
}

bool AlgorithmTester::testLevels() const {
    const SBRunAgentRef agent = SBRunAdapterGetAgent(m_runAdapter);
    SBRunAdapterReset(m_runAdapter);

    while (SBRunAdapterMoveNext(m_runAdapter)) {
        SBUInteger start = agent->offset;
        SBUInteger end = start + agent->length - 1;
        SBLevel level = agent->level;

        if (end >= m_charCount) {
            if (Configuration::DISPLAY_ERROR_DETAILS) {
                cout << "Test failed due to invalid run indexes." << endl;
                cout << "  Text Length: " << m_charCount << endl;
                cout << "  Run Start Index: " << start << endl;
                cout << "  Run End Index: " << end << endl;
            }
            return false;
        }

        uint8_t oldLevel = m_paragraphLevel;
        for (size_t i = start; i--;) {
            if (m_levels->at(i) != LEVEL_X) {
                oldLevel = m_levels->at(i);
                break;
            }
        }

        for (size_t i = start; i <= end; i++) {
            if (m_levels->at(i) == LEVEL_X) {
                if (level != oldLevel) {
                    cout << "Warning: Level X should be equal to previous level." << endl;
                }
                continue;
            }

            if (m_levels->at(i) != level) {
                if (Configuration::DISPLAY_ERROR_DETAILS) {
                    cout << "Test failed due to level mismatch." << endl;
                    cout << "  Text Index: " << i << endl;
                    cout << "  Discovered Level: " << (int)level << endl;
                    cout << "  Expected Level: " << (int)m_levels->at(i) << endl;
                }
                return false;
            }

            oldLevel = m_levels->at(i);
        }
    }

    return true;
}

bool AlgorithmTester::testOrder() const {
    bool passed = true;
    size_t lgcIndex = 0;    // Logical Index (incremented from zero to char count)
    size_t ordIndex = 0;    // Order Index (not incremented when level x is found)
    size_t dcvIndex = 0;    // Discovered Visual Index
    size_t expIndex = 0;    // Expected Visual Index

    SBRunAgentRef agent = SBRunAdapterGetAgent(m_runAdapter);
    SBRunAdapterReset(m_runAdapter);

    while (SBRunAdapterMoveNext(m_runAdapter)) {
        SBUInteger start = agent->offset;
        SBUInteger end = start + agent->length - 1;
        SBLevel level = agent->level;

        if (level & 1) {
            for (size_t i = start, dcvIndex = end; i <= end; i++, dcvIndex--) {
                lgcIndex++;

                if (m_levels->at(dcvIndex) == LEVEL_X) {
                    continue;
                }

                expIndex = m_order->at(ordIndex++);
                if (expIndex != dcvIndex) {
                    passed = false;
                    break;
                }
            }
        } else {
            for (dcvIndex = start; dcvIndex <= end; dcvIndex++) {
                lgcIndex++;

                if (m_levels->at(dcvIndex) == LEVEL_X) {
                    continue;
                }

                expIndex = m_order->at(ordIndex++);
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

bool AlgorithmTester::testMirrors() const {
    const SBMirrorAgentRef agent = SBMirrorLocatorGetAgent(m_mirrorLocator);
    SBMirrorLocatorReset(m_mirrorLocator);

    size_t locatedMirrors = 0;

    while (SBMirrorLocatorMoveNext(m_mirrorLocator)) {
        if (agent->index < m_charCount) {
            if (agent->mirror == m_genMirrors[agent->index]) {
                ++locatedMirrors;
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
                         << setw(4) << m_genMirrors[agent->index] << endl
                         << nouppercase << dec << setfill('\0');
                }

                return false;
            }
        } else {
            if (Configuration::DISPLAY_ERROR_DETAILS) {
                cout << "Test failed due to invalid mirror index." << endl;
                cout << "  Text Length: " << m_charCount;
                cout << "  Located Index: " << agent->index << endl;
            }

            return false;
        }
    }

    if (locatedMirrors != m_mirrorCount) {
        if (Configuration::DISPLAY_ERROR_DETAILS) {
            cout << "Test failed due to mismatch in mirror count." << endl;
            cout << "  Discovered Mirrors: " << locatedMirrors << endl;
            cout << "  Expected Mirrors: " << m_mirrorCount << endl;
        }

        return false;
    }

    return true;
}

bool AlgorithmTester::conductTest() {
    bool passed = true;

    SBCodepointSequenceLoadUTF32Buffer(m_codepointSequence, m_genChars, m_charCount);

    SBParagraphRef paragraph = SBParagraphCreate(m_codepointSequence, m_inputLevel);
    SBLevel paragraphlevel = SBParagraphGetBaseLevel(paragraph);
    if (m_paragraphLevel == LEVEL_X) {
        m_paragraphLevel = paragraphlevel;
    }

    if (paragraphlevel == m_paragraphLevel) {
        SBLineRef line = SBLineCreateWithParagraph(paragraph, 0, m_charCount);
        SBRunAdapterLoadLine(m_runAdapter, line);

        passed &= testLevels();
        passed &= testOrder();

        if (m_bidiMirroring) {
            loadMirrors();
            SBMirrorLocatorLoadLine(m_mirrorLocator, line, (void *)m_genChars);
            passed &= testMirrors();
        }

        SBLineRelease(line);
    } else {
        if (Configuration::DISPLAY_ERROR_DETAILS) {
            cout << "Test failed due to paragraph level mismatch." << endl;
            cout << "  Discovered Paragraph Level: " << (int)paragraphlevel << endl;
            cout << "  Expected Paragraph Level: " << (int)m_paragraphLevel << endl;
        }

        passed &= false;
    }

    SBParagraphRelease(paragraph);
    
    return passed;
}

void AlgorithmTester::displayBidiTestCase() const {
    if (Configuration::DISPLAY_TEST_CASE) {
        auto &testCase = m_bidiTest->testCase();

        cout << "Test # " << testCounter << ':' << endl;

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
        if (testCase.order.size() == 0) {
            cout << "Empty";
        } else {
            for (auto index : testCase.order) {
                cout << index << '\t';
            }
        }
        cout << endl;
    }
}

void AlgorithmTester::displayBidiCharacterTestCase() const {
    if (Configuration::DISPLAY_TEST_CASE) {
        auto &testCase = m_bidiCharacterTest->testCase();

        cout << "Test # " << testCounter << endl;

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
        if (testCase.order.size() == 0) {
            cout << "Empty";
        } else {
            for (auto index : testCase.order) {
                cout << index << '\t';
            }
        }
        cout << endl;
    }
}

void AlgorithmTester::analyzeBidiTest() {
    auto &testCase = m_bidiTest->testCase();
    m_bidiTest->reset();

    for (; m_bidiTest->fetchNext(); testCounter++) {
        displayBidiTestCase();

        m_levels = &testCase.levels;
        m_order = &testCase.order;

        bool passed = true;
        loadCharacters(testCase.types);

        if (testCase.directions & BidiTest::ParagraphDirection::Auto) {
            m_inputLevel = SBLevelDefaultLTR;
            m_paragraphLevel = LEVEL_X;
            passed &= conductTest();
        }

        if (testCase.directions & BidiTest::ParagraphDirection::LTR) {
            loadMirrors();
            m_inputLevel = 0;
            m_paragraphLevel = LEVEL_X;
            passed &= conductTest();
        }

        if (testCase.directions & BidiTest::ParagraphDirection::RTL) {
            m_inputLevel = 1;
            m_paragraphLevel = LEVEL_X;
            passed &= conductTest();
        }

        if (!passed) {
            failCounter++;
        }
    }
}

void AlgorithmTester::analyzeBidiCharacterTest() {
    const BidiCharacterTest::TestCase &testCase = m_bidiCharacterTest->testCase();
    m_bidiCharacterTest->reset();

    for (; m_bidiCharacterTest->fetchNext(); testCounter++) {
        displayBidiCharacterTestCase();

        m_paragraphLevel = testCase.paragraphLevel;
        m_levels = &testCase.levels;
        m_order = &testCase.order;

        loadCharacters(testCase.text);

        switch (testCase.paragraphDirection) {
            case BidiCharacterTest::ParagraphDirection::LTR:
                m_inputLevel = 0;
                break;

            case BidiCharacterTest::ParagraphDirection::RTL:
                m_inputLevel = 1;
                break;

            default:
                m_inputLevel = SBLevelDefaultLTR;
                break;
        }
        
        if (!conductTest()) {
            failCounter++;
        }
    }
}
