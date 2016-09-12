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

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <string>

#include "UnicodeData.h"

using namespace std;
using namespace SheenBidi::Parser;

static const string FILE_UNICODE_DATA = "UnicodeData.txt";
static const string STRING_EMPTY = "";

static inline void initializeBidiClassNames(vector<string> &obj) {
    obj.reserve(25);
    obj.push_back(STRING_EMPTY);
}

static inline const string &getBidiClassName(vector<string> &obj, uint8_t classNumber) {
    return obj.at(classNumber);
}

static inline uint8_t getBidiClassNumber(vector<string> &obj, const string &className) {
    auto begin = obj.begin();
    auto end = obj.end();
    auto match = find(begin, end, className);
    if (match != end) {
        return distance(begin, match);
        
    }

    uint8_t number = obj.size();
    obj.push_back(className);

    return number;
}

UnicodeData::UnicodeData(const string &directory) :
    m_bidiClassNumbers(0x200000),
    m_firstCodePoint(0),
    m_lastCodePoint(0)
{
    initializeBidiClassNames(m_bidiClassNames);

    ifstream stream(directory + "/" + FILE_UNICODE_DATA, ios::binary);
    string bidiClassName;

    while (!stream.eof()) {
        uint32_t codePoint;
        stream >> hex >> setw(6) >> codePoint;

        m_lastCodePoint = codePoint;

        stream.ignore(128, ';');
        stream.ignore(128, ';');
        stream.ignore(128, ';');
        stream.ignore(128, ';');

        getline(stream, bidiClassName, ';');
        m_bidiClassNumbers[codePoint] = getBidiClassNumber(m_bidiClassNames, bidiClassName);

        stream.ignore(1024, '\n');
    }
}

uint32_t UnicodeData::firstCodePoint() const {
    return m_firstCodePoint;
}

uint32_t UnicodeData::lastCodePoint() const {
    return m_lastCodePoint;
}

const string &UnicodeData::bidiClassForCodePoint(uint32_t codePoint) const {
    if (codePoint <= m_lastCodePoint) {
        return m_bidiClassNames.at(m_bidiClassNumbers.at(codePoint));
    }

    return STRING_EMPTY;
}
