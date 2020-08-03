/*
 * Copyright (C) 2020 Muhammad Tayyab Akram
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
#include "DerivedBidiClass.h"

using namespace std;
using namespace SheenBidi::Parser;

static const string FILE_DERIVED_BIDI_CLASS = "DerivedBidiClass.txt";
static const string BIDI_CLASS_MISSING = "L";

static inline void initializeBidiClassNames(vector<string> &obj) {
    obj.reserve(25);
    obj.push_back(BIDI_CLASS_MISSING);
}

static inline uint8_t getBidiClassNumber(vector<string> &obj, const string &bidiClass) {
    auto begin = obj.begin();
    auto end = obj.end();
    auto match = find(begin, end, bidiClass);
    if (match != end) {
        return static_cast<uint8_t>(distance(begin, match));
    }

    uint8_t number = static_cast<uint8_t>(obj.size());
    obj.push_back(bidiClass);

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

static inline char *readBidiClass(char *field, string *name) {
    char *start = field;
    while (*start++ != ' ');

    char *end = start;
    while (*end++ != ' ');

    size_t length = static_cast<size_t>(distance(start, end) - 1);
    *name = string(start, length);

    return end;
}

DerivedBidiClass::DerivedBidiClass(const string &directory) :
    m_firstCodePoint(0),
    m_lastCodePoint(0),
    m_classNames(),
    m_classNumbers(0x110000)
{
    initializeBidiClassNames(m_classNames);
    ifstream stream(directory + "/" + FILE_DERIVED_BIDI_CLASS, ios::in);

    string versionLine;
    getline(stream, versionLine);
    m_version = new UnicodeVersion(versionLine);

    string line;
    while (getline(stream, line)) {
        if (!line.empty() && line[0] != '#') {
            uint32_t firstCodePoint = 0;
            uint32_t lastCodePoint = 0;
            string bidiClass;

            char *field = &line[0];
            field = readCodePointRange(field, &firstCodePoint, &lastCodePoint);
            field = readBidiClass(field, &bidiClass);

            uint8_t classNumber = getBidiClassNumber(m_classNames, bidiClass);
            for (uint32_t codePoint = firstCodePoint; codePoint <= lastCodePoint; codePoint++) {
                m_classNumbers[codePoint] = classNumber;
            }

            if (lastCodePoint > m_lastCodePoint) {
                m_lastCodePoint = lastCodePoint;
            }
        }
    }
}

DerivedBidiClass::~DerivedBidiClass() {
    delete m_version;
}

uint32_t DerivedBidiClass::firstCodePoint() const {
    return m_firstCodePoint;
}

uint32_t DerivedBidiClass::lastCodePoint() const {
    return m_lastCodePoint;
}

UnicodeVersion &DerivedBidiClass::version() const {
    return *m_version;
}

const string &DerivedBidiClass::bidiClassForCodePoint(uint32_t codePoint) const {
    if (codePoint <= m_lastCodePoint) {
        return m_classNames.at(m_classNumbers.at(codePoint));
    }

    return BIDI_CLASS_MISSING;
}
