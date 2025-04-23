/*
 * Copyright (C) 2025 Muhammad Tayyab Akram
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
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

#include "UnicodeVersion.h"
#include "DerivedGeneralCategory.h"

using namespace std;
using namespace SheenBidi::Parser;

static const string FILE_DERIVED_GENERAL_CATEGORY = "DerivedGeneralCategory.txt";
static const string GENERAL_CATEGORY_DEFAULT = "Cn";

static inline void initializeGeneralCategoryNames(vector<string> &obj) {
    obj.reserve(32);
    obj.push_back(GENERAL_CATEGORY_DEFAULT);
}

static inline uint8_t getGeneralCategoryNumber(vector<string> &obj, const string &generalCategory) {
    auto begin = obj.begin();
    auto end = obj.end();
    auto match = find(begin, end, generalCategory);
    if (match != end) {
        return static_cast<uint8_t>(distance(begin, match));
    }

    uint8_t number = static_cast<uint8_t>(obj.size());
    obj.push_back(generalCategory);

    return number;
}

static inline char *readCodePointRange(char *field, uint32_t *first, uint32_t *last) {
    *first = static_cast<uint32_t>(strtoul(field, &field, 16));

    if (*field == '.') {
        *last = static_cast<uint32_t>(strtoul(field + 2, &field, 16));
    } else {
        *last = *first;
    }

    while (*field++ != ';');

    return field;
}

static inline char *readGeneralCategory(char *field, string *name) {
    char *start = field;
    while (*start++ != ' ');

    char *end = start;
    while (*end++ != ' ');

    size_t length = static_cast<size_t>(distance(start, end) - 1);
    *name = string(start, length);

    return end;
}

DerivedGeneralCategory::DerivedGeneralCategory(const string &directory) :
    m_firstCodePoint(0),
    m_lastCodePoint(0),
    m_categoryNumbers(0x110000)
{
    initializeGeneralCategoryNames(m_categoryNames);

    auto filePath = directory + "/" + FILE_DERIVED_GENERAL_CATEGORY;
    ifstream stream(filePath, ios::in);
    if (!stream.is_open()) {
        throw runtime_error("Failed to open file: " + filePath);
    }

    string versionLine;
    getline(stream, versionLine);
    m_version = new UnicodeVersion(versionLine);

    string line;
    while (getline(stream, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        uint32_t firstCodePoint = 0;
        uint32_t lastCodePoint = 0;
        string generalCategory;

        char *field = &line[0];
        field = readCodePointRange(field, &firstCodePoint, &lastCodePoint);
        field = readGeneralCategory(field, &generalCategory);

        uint8_t classNumber = getGeneralCategoryNumber(m_categoryNames, generalCategory);
        for (uint32_t codePoint = firstCodePoint; codePoint <= lastCodePoint; codePoint++) {
            m_categoryNumbers[codePoint] = classNumber;
        }

        if (lastCodePoint > m_lastCodePoint) {
            m_lastCodePoint = lastCodePoint;
        }
    }
}

DerivedGeneralCategory::~DerivedGeneralCategory() {
    delete m_version;
}

uint32_t DerivedGeneralCategory::firstCodePoint() const {
    return m_firstCodePoint;
}

uint32_t DerivedGeneralCategory::lastCodePoint() const {
    return m_lastCodePoint;
}

UnicodeVersion &DerivedGeneralCategory::version() const {
    return *m_version;
}

const string &DerivedGeneralCategory::generalCategoryForCodePoint(uint32_t codePoint) const {
    if (codePoint <= m_lastCodePoint) {
        return m_categoryNames.at(m_categoryNumbers.at(codePoint));
    }

    return GENERAL_CATEGORY_DEFAULT;
}
