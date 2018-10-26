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
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

#include "UnicodeVersion.h"
#include "Scripts.h"

using namespace std;
using namespace SheenBidi::Parser;

static const string FILE_SCRIPTS = "Scripts.txt";
static const string SCRIPT_UNKNOWN = "Unknown";
static const string SCRIPT_COMMON = "Common";
static const string SCRIPT_INHERITED = "Inherited";

static inline void initializeScriptNames(vector<string> &obj) {
    obj.reserve(25);
    obj.push_back(SCRIPT_UNKNOWN);
    obj.push_back(SCRIPT_COMMON);
    obj.push_back(SCRIPT_INHERITED);
}

static inline uint8_t getScriptNumber(vector<string> &obj, const string &scriptName) {
    auto begin = obj.begin();
    auto end = obj.end();
    auto match = find(begin, end, scriptName);
    if (match != end) {
        return static_cast<uint8_t>(distance(begin, match));
    }

    uint8_t number = static_cast<uint8_t>(obj.size());
    obj.push_back(scriptName);

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

static inline char *readScriptName(char *field, string *name) {
    char *start = field;
    while (*start++ != ' ');

    char *end = start;
    while (*end++ != ' ');

    size_t length = static_cast<size_t>(distance(start, end) - 1);
    *name = string(start, length);

    return end;
}

Scripts::Scripts(const string &directory) :
    m_firstCodePoint(0),
    m_lastCodePoint(0),
    m_scriptNames(),
    m_scriptNumbers(0x200000)
{
    initializeScriptNames(m_scriptNames);
    ifstream stream(directory + "/" + FILE_SCRIPTS, ios::in);

    string versionLine;
    getline(stream, versionLine);
    m_version = new UnicodeVersion(versionLine);

    string line;
    while (getline(stream, line)) {
        if (!line.empty() && line[0] != '#') {
            uint32_t firstCodePoint = 0;
            uint32_t lastCodePoint = 0;
            string scriptName;

            char *field = &line[0];
            field = readCodePointRange(field, &firstCodePoint, &lastCodePoint);
            field = readScriptName(field, &scriptName);

            uint8_t scriptNumber = getScriptNumber(m_scriptNames, scriptName);
            for (uint32_t codePoint = firstCodePoint; codePoint <= lastCodePoint; codePoint++) {
                m_scriptNumbers[codePoint] = scriptNumber;
            }

            if (lastCodePoint > m_lastCodePoint) {
                m_lastCodePoint = lastCodePoint;
            }
        }
    }
}

Scripts::~Scripts() {
    delete m_version;
}

uint32_t Scripts::firstCodePoint() const {
    return m_firstCodePoint;
}

uint32_t Scripts::lastCodePoint() const {
    return m_lastCodePoint;
}

UnicodeVersion &Scripts::version() const {
    return *m_version;
}

const string &Scripts::scriptForCodePoint(uint32_t codePoint) const {
    if (codePoint <= m_lastCodePoint) {
        return m_scriptNames.at(m_scriptNumbers.at(codePoint));
    }

    return SCRIPT_UNKNOWN;
}
