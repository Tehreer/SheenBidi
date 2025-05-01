/*
 * Copyright (C) 2018-2025 Muhammad Tayyab Akram
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
#include <string>

#include "DataFile.h"
#include "UnicodeVersion.h"
#include "PropertyValueAliases.h"

using namespace std;
using namespace SheenBidi::Parser;

static const string FILE_PROPERTY_VALUE_ALIASES = "PropertyValueAliases.txt";
static const string PROPERTY_CANONICAL_COMBINING_CLASS = "ccc";
static const string PROPERTY_BIDI_CLASS = "bc";
static const string PROPERTY_SCRIPT = "sc";

PropertyValueAliases::PropertyValueAliases(const string &directory) :
    DataFile(directory, FILE_PROPERTY_VALUE_ALIASES)
{
    string line;
    if (readLine(line)) {
        getVersion(line, m_version);
    }

    while (readLine(line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        string property;
        string numeric;
        string shortName;
        string longName;

        size_t index = 0;
        index = getField(line, index, FieldTerminator::SpaceOrSemicolon, property);

        if (property != PROPERTY_CANONICAL_COMBINING_CLASS) {
            index = getField(line, index, FieldTerminator::SpaceOrSemicolon, shortName);
            index = getField(line, index, FieldTerminator::SpaceOrSemicolon, longName);
        } else {
            index = getField(line, index, FieldTerminator::SpaceOrSemicolon, numeric);
            index = getField(line, index, FieldTerminator::SpaceOrSemicolon, shortName);
            index = getField(line, index, FieldTerminator::SpaceOrSemicolon, longName);
        }

        if (property == PROPERTY_BIDI_CLASS) {
            m_bidiClassMap[longName] = shortName;
        } else if (property == PROPERTY_SCRIPT) {
            m_scriptMap[longName] = shortName;
        }
    }
}

const UnicodeVersion &PropertyValueAliases::version() const {
    return m_version;
}

const std::string &PropertyValueAliases::abbreviationForBidiClass(const std::string &bidiClass) const {
    return m_bidiClassMap.at(bidiClass);
}

const std::string &PropertyValueAliases::abbreviationForScript(const string &scriptName) const {
    return m_scriptMap.at(scriptName);
}
