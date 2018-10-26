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

#include <cstdint>
#include <fstream>
#include <map>
#include <string>

#include "UnicodeVersion.h"
#include "PropertyValueAliases.h"

using namespace std;
using namespace SheenBidi::Parser;

static const string FILE_PROPERTY_VALUE_ALIASES = "PropertyValueAliases.txt";

static inline char *readPropertyName(char *field, string *name) {
    char *start = field;
    for (; *field != ' '; field++);

    size_t length = static_cast<size_t>(distance(start, field));
    *name = string(start, length);
    while (*field++ != ';');

    return field;
}

static inline char *readAbbreviation(char *field, string *abbr) {
    for (; *field == ' '; field++);

    char *start = field;
    for (; *field != ' '; field++);

    size_t length = static_cast<size_t>(distance(start, field));
    *abbr = string(start, length);
    while (*field++ != ';');

    return field;
}

static inline char *readLongName(char *field, string *name) {
    for (; *field == ' '; field++);

    char *start = field;
    for (; *field != ' ' && *field != '\0'; field++);

    size_t length = static_cast<size_t>(distance(start, field));
    *name = string(start, length);

    return field;
}

PropertyValueAliases::PropertyValueAliases(const string &directory) :
    m_firstCodePoint(0),
    m_lastCodePoint(0),
    m_scriptMap()
{
    ifstream stream(directory + "/" + FILE_PROPERTY_VALUE_ALIASES, ios::in);

    string versionLine;
    getline(stream, versionLine);
    m_version = new UnicodeVersion(versionLine);

    string line;
    while (getline(stream, line)) {
        if (!line.empty() && line[0] != '#') {
            string propertyName;
            string abbreviation;
            string longName;

            char *field = &line[0];
            field = readPropertyName(field, &propertyName);

            if (propertyName == "sc") {
                field = readAbbreviation(field, &abbreviation);
                field = readLongName(field, &longName);

                m_scriptMap[longName] = abbreviation;
            }
        }
    }
}

PropertyValueAliases::~PropertyValueAliases() {
    delete m_version;
}

uint32_t PropertyValueAliases::firstCodePoint() const {
    return m_firstCodePoint;
}

uint32_t PropertyValueAliases::lastCodePoint() const {
    return m_lastCodePoint;
}

UnicodeVersion &PropertyValueAliases::version() const {
    return *m_version;
}

const std::string &PropertyValueAliases::abbreviationForScript(const string &scriptName) const {
    return m_scriptMap.at(scriptName);
}
