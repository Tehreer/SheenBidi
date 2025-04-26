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

#include <cstdint>
#include <fstream>
#include <string>

#include "BidiCharacterTest.h"

using namespace std;
using namespace SheenBidi::Parser;

static const string BIDI_CHARACTER_TEST_FILE = "BidiCharacterTest.txt";

static inline void initializeTestCase(BidiCharacterTest::TestCase &testCase) {
    const int DefaultLength = 32;

    testCase.text.reserve(DefaultLength);
    testCase.levels.reserve(DefaultLength);
    testCase.order.reserve(DefaultLength);
}

static inline void clearText(BidiCharacterTest::TestCase &testCase) {
    testCase.text.clear();
}

static inline void addCodePoint(BidiCharacterTest::TestCase &testCase, uint32_t codePoint) {
    testCase.text.push_back(codePoint);
}

static inline size_t readText(BidiCharacterTest::TestCase &testCase, const string &line, size_t index) {
    clearText(testCase);

    uint32_t codePoint = 0;
    uint32_t shift = 12;

    for (; index < line.length(); index++) {
        if (line[index] == ';') {
            break;
        }

        if (line[index] == ' ') {
            addCodePoint(testCase, codePoint);
            codePoint = 0;
            shift = 12;
        } else {
            uint32_t digit = (line[index] <= '9' ? line[index] - '0' : line[index] - 'A' + 10);
            codePoint += digit << shift;
            shift -= 4;
        }
    }
    addCodePoint(testCase, codePoint);

    return (index + 1);
}

static inline size_t readParagraphDirection(BidiCharacterTest::TestCase &testCase, const string &line, size_t index) {
    testCase.paragraphDirection = (BidiCharacterTest::ParagraphDirection)(line.at(index) - '0');
    return (index + 2);
}

static inline size_t readParagraphLevel(BidiCharacterTest::TestCase &testCase, const string &line, size_t index) {
    testCase.paragraphLevel = (line.at(index) - '0');
    return (index + 2);
}

static inline void clearLevels(BidiCharacterTest::TestCase &testCase) {
    testCase.levels.clear();
}

static inline void addLevel(BidiCharacterTest::TestCase &testCase, uint8_t level) {
    testCase.levels.push_back(level);
}

static inline size_t readLevels(BidiCharacterTest::TestCase &testCase, const string &line, size_t index) {
    clearLevels(testCase);

    uint8_t level = 0;

    for (; index < line.length(); index++) {
        if (line[index] == ';') {
            break;
        }

        if (line[index] == ' ') {
            addLevel(testCase, level);
            level = 0;
        } else if (line[index] == 'x') {
            level = BidiCharacterTest::LEVEL_X;
        } else {
            level *= 10;
            level += line[index] - '0';
        }
    }
    addLevel(testCase, level);

    return (index + 1);
}

static inline void clearOrder(BidiCharacterTest::TestCase &testCase) {
    testCase.order.clear();
}

static inline void addOrderIndex(BidiCharacterTest::TestCase &testCase, size_t index) {
    testCase.order.push_back(index);
}

static inline size_t readOrder(BidiCharacterTest::TestCase &testCase, const string &line, size_t index) {
    clearOrder(testCase);

    size_t order = 0;

    for (; index < line.length(); index++) {
        if (line[index] == ' ') {
            addOrderIndex(testCase, order);
            order = 0;
        } else {
            order *= 10;
            order += line[index] - '0';
        }
    }
    addOrderIndex(testCase, order);

    return index;
}

BidiCharacterTest::BidiCharacterTest(const string &directory) {
    auto filePath = directory + "/" + BIDI_CHARACTER_TEST_FILE;
    m_stream.open(filePath, ios::in);
    if (!m_stream.is_open()) {
        throw runtime_error("Failed to open file: " + filePath);
    }

    initializeTestCase(m_testCase);
}

BidiCharacterTest::~BidiCharacterTest() {
    m_stream.close();
}

const BidiCharacterTest::TestCase &BidiCharacterTest::testCase() const {
    return m_testCase;
}

bool BidiCharacterTest::fetchNext() {
    while (getline(m_stream, m_line)) {
        if (m_line.empty() || m_line[0] == '#') {
            continue;
        }

        size_t index = 0;
        index = readText(m_testCase, m_line, index);
        index = readParagraphDirection(m_testCase, m_line, index);
        index = readParagraphLevel(m_testCase, m_line, index);
        index = readLevels(m_testCase, m_line, index);
        index = readOrder(m_testCase, m_line, index);

        return true;
    }

    return false;
}

void BidiCharacterTest::reset() {
    m_stream.seekg(0);
}
