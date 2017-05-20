/*
 * Copyright (C) 2015 Muhammad Tayyab Akram
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

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "BidiCharacterTest.h"

using namespace std;
using namespace SheenBidi::Parser;

static const string BIDI_CHARACTER_TEST_FILE = "BidiCharacterTest.txt";

static inline void initializeTestCase(BidiCharacterTest::TestCase &testCase)
{
    const int DefaultLength = 32;

    testCase.text.reserve(DefaultLength);
    testCase.levels.reserve(DefaultLength);
    testCase.order.reserve(DefaultLength);
}

static inline void clearText(BidiCharacterTest::TestCase &testCase) {
    testCase.text.clear();
}

static inline void addTextChar(BidiCharacterTest::TestCase &testCase, uint32_t character) {
    testCase.text.push_back(character);
}

static inline char *readText(BidiCharacterTest::TestCase &testCase, char *field) {
    clearText(testCase);

    uint32_t codePoint = 0;
    uint32_t shift = 12;

    for (; *field != ';'; ++field) {
        if (*field == ' ') {
            addTextChar(testCase, codePoint);
            codePoint = 0;
            shift = 12;
        } else {
            uint32_t digit = (*field <= '9' ? *field - '0' : *field - 'A' + 10);
            codePoint += digit << shift;
            shift -= 4;
        }
    }
    addTextChar(testCase, codePoint);

    return (field + 1);
}

static inline char *readParagraphDirection(BidiCharacterTest::TestCase &testCase, char *field) {
    testCase.paragraphDirection = (BidiCharacterTest::ParagraphDirection)(*field - '0');
    return (field + 2);
}

static inline char *readParagraphLevel(BidiCharacterTest::TestCase &testCase, char *field) {
    testCase.paragraphLevel = (*field - '0');
    return (field + 2);
}

static inline void clearLevels(BidiCharacterTest::TestCase &testCase) {
    testCase.levels.clear();
}

static inline void addLevel(BidiCharacterTest::TestCase &testCase, uint8_t level) {
    testCase.levels.push_back(level);
}

static inline char *readLevels(BidiCharacterTest::TestCase &testCase, char *field) {
    clearLevels(testCase);

    uint8_t level = 0;

    for (; *field != ';'; ++field) {
        if (*field == ' ') {
            addLevel(testCase, level);
            level = 0;
        } else if (*field == 'x') {
            level = BidiCharacterTest::LEVEL_X;
        } else {
            level *= 10;
            level += *field - '0';
        }
    }
    addLevel(testCase, level);

    return (field + 1);
}

static inline void clearOrder(BidiCharacterTest::TestCase &testCase) {
    testCase.order.clear();
}

static inline void addOrderIndex(BidiCharacterTest::TestCase &testCase, size_t index) {
    testCase.order.push_back(index);
}

static inline char *readOrder(BidiCharacterTest::TestCase &testCase, char *field) {
    clearOrder(testCase);

    size_t index = 0;

    for (; *field != '\0'; ++field) {
        if (*field == ' ') {
            addOrderIndex(testCase, index);
            index = 0;
        } else {
            index *= 10;
            index += *field - '0';
        }
    }
    addOrderIndex(testCase, index);

    return NULL;
}

BidiCharacterTest::BidiCharacterTest(const string &directory) :
    m_Stream(directory + "/" + BIDI_CHARACTER_TEST_FILE, ios::binary)
{
    initializeTestCase(m_TestCase);
}

BidiCharacterTest::~BidiCharacterTest() {
    m_Stream.close();
}

const BidiCharacterTest::TestCase &BidiCharacterTest::testCase() const {
    return m_TestCase;
}

bool BidiCharacterTest::fetchNext() {
    const int BufferSize = 2048;
    char line[BufferSize];

    while (!m_Stream.eof()) {
        m_Stream.getline(line, BufferSize);
        
        if (line[0] != '\0' && line[0] != '#') {
            char *field = line;

            field = readText(m_TestCase, field);
            field = readParagraphDirection(m_TestCase, field);
            field = readParagraphLevel(m_TestCase, field);
            field = readLevels(m_TestCase, field);
            field = readOrder(m_TestCase, field);

            return true;
        }
    }

    return false;
}

void BidiCharacterTest::reset() {
    m_Stream.seekg(0);
}
