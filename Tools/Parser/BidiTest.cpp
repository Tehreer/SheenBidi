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

#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>

#include "BidiTest.h"

using namespace std;
using namespace SheenBidi::Parser;

static const string FILE_BIDI_TEST = "BidiTest.txt";

static inline char *skipSpaces(char *line) {
    while (*line == ' ' || *line == '\t') {
        ++line;
    }
    
    return line;
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

static inline void readLevels(BidiTest::TestCase &testCase, char *line) {
    uint8_t level = 0;

    line = skipSpaces(line);
    clearLevels(testCase);

    for (; *line != '\0'; ++line) {
        if (*line == ' ') {
            addLevel(testCase, level);
            level = 0;
        } else if (*line == 'x') {
            level = BidiTest::LEVEL_X;
        } else {
            level *= 10;
            level += *line - '0';
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

static inline void readVisualOrder(BidiTest::TestCase &testCase, char *line) {
    size_t index = 0;

    line = skipSpaces(line);
    clearOrder(testCase);

    for (; *line != '\0'; ++line) {
        if (*line == ' ') {
            addOrderIndex(testCase, index);
            index = 0;
        } else {
            index *= 10;
            index += *line - '0';
        }
    }
    addOrderIndex(testCase, index);
}

static inline void clearTypes(BidiTest::TestCase &testCase) {
    testCase.types.clear();
}

static inline void addType(BidiTest::TestCase &testCase, const char *type) {
    testCase.types.push_back(string(type));
}

static inline void readData(BidiTest::TestCase &testCase, char *line) {
    clearTypes(testCase);

    char type[4];
    char *var = type;

    for (; *line != ';'; ++line) {
        if (*line == ' ') {
            *var = '\0';
            addType(testCase, type);

            var = type;
        } else {
            *var++ = *line;
        }
    }
    *var = '\0';
    addType(testCase, type);

    testCase.directions = (BidiTest::ParagraphDirection)(*(line + 2) - '0');
}

BidiTest::BidiTest(const string &directory) :
    m_stream(directory + "/" + FILE_BIDI_TEST, ios::binary)
{
    initializeTestCase(m_testCase);
}

BidiTest::~BidiTest() {
    m_stream.close();
}

const BidiTest::TestCase &BidiTest::testCase() const {
    return m_testCase;
}

bool BidiTest::fetchNext() {
    const int BufferSize = 512;
    char line[BufferSize];

    while (!m_stream.eof()) {
        m_stream.getline(line, BufferSize);

        if (line[0] != '\0' && line[0] != '#') {
            if (line[0] == '@') {
                if (strstr(line, "Levels:")) {
                    readLevels(m_testCase, line + 8);
                } else if (strstr(line, "Reorder:")) {
                    readVisualOrder(m_testCase, line + 9);
                }
            } else {
                readData(m_testCase, line);
                return true;
            }
        }
    }

    return false;
}

void BidiTest::reset() {
    m_stream.seekg(0);
}
