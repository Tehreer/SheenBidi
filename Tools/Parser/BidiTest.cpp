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

#include "BidiTest.h"

using namespace std;
using namespace SheenBidi::Parser;

static const string FILE_BIDI_TEST = "BidiTest.txt";

static inline size_t skipSpaces(const string &line, size_t index) {
    return line.find_first_not_of(" \t", index);
}

static inline void initializeTestCase(BidiTest::TestCase &testCase) {
    const int DefaultLength = 96;

    testCase.types.reserve(DefaultLength);
    testCase.levels.reserve(DefaultLength);
    testCase.order.reserve(DefaultLength);
}

static inline void clearLevels(BidiTest::TestCase &testCase) {
    testCase.levels.clear();
}

static inline void addLevel(BidiTest::TestCase &testCase, uint8_t level) {
    testCase.levels.push_back(level);
}

static inline void readLevels(BidiTest::TestCase &testCase, const string &line, size_t index) {
    uint8_t level = 0;

    index = skipSpaces(line, index);
    clearLevels(testCase);

    for (; index < line.length(); index++) {
        if (line[index] == ' ') {
            addLevel(testCase, level);
            level = 0;
        } else if (line[index] == 'x') {
            level = BidiTest::LEVEL_X;
        } else {
            level *= 10;
            level += line[index] - '0';
        }
    }
    addLevel(testCase, level);
}

static inline void clearOrder(BidiTest::TestCase &testCase) {
    testCase.order.clear();
}

static inline void addOrderIndex(BidiTest::TestCase &testCase, size_t index) {
    testCase.order.push_back(index);
}

static inline void readVisualOrder(BidiTest::TestCase &testCase, const string &line, size_t index) {
    size_t order = 0;

    index = skipSpaces(line, index);
    clearOrder(testCase);

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
}

static inline void clearTypes(BidiTest::TestCase &testCase) {
    testCase.types.clear();
}

static inline void addType(BidiTest::TestCase &testCase, const string &type) {
    testCase.types.push_back(type);
}

static inline void readData(BidiTest::TestCase &testCase, const string &line) {
    clearTypes(testCase);

    size_t index = 0;
    string type;

    for (; index < line.length(); index++) {
        if (line[index] == ';') {
            index += 1;
            break;
        }

        if (line[index] == ' ') {
            addType(testCase, type);

            type.clear();
        } else {
            type += line[index];
        }
    }
    addType(testCase, type);

    index = skipSpaces(line, index);
    testCase.directions = (BidiTest::ParagraphDirection)(line.at(index) - '0');
}

BidiTest::BidiTest(const string &directory) {
    auto filePath = directory + "/" + FILE_BIDI_TEST;
    m_stream.open(filePath, ios::in);
    if (!m_stream.is_open()) {
        throw runtime_error("Failed to open file: " + filePath);
    }

    initializeTestCase(m_testCase);
}

BidiTest::~BidiTest() {
    m_stream.close();
}

const BidiTest::TestCase &BidiTest::testCase() const {
    return m_testCase;
}

bool BidiTest::fetchNext() {
    while (getline(m_stream, m_line)) {
        if (m_line.empty() || m_line[0] == '#') {
            continue;
        }

        if (m_line[0] == '@') {
            string levels = "Levels:";
            string reorder = "Reorder:";

            if (m_line.compare(1, levels.length(), levels) == 0) {
                readLevels(m_testCase, m_line, levels.length() + 1);
            } else if (m_line.compare(1, reorder.length(), reorder) == 0) {
                readVisualOrder(m_testCase, m_line, reorder.length() + 1);
            }
        } else {
            readData(m_testCase, m_line);
            return true;
        }
    }

    return false;
}

void BidiTest::reset() {
    m_stream.seekg(0);
}
