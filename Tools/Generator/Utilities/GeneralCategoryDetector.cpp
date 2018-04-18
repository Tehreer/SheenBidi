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
#include <map>
#include <string>
#include <vector>

#include <Parser/UnicodeData.h>

#include "GeneralCategoryDetector.h"

using namespace std;
using namespace SheenBidi::Parser;
using namespace SheenBidi::Generator::Utilities;

GeneralCategoryDetector::GeneralCategoryDetector(const UnicodeData &unicodeData)
    : m_unicodeData(unicodeData)
    , m_numbers(unicodeData.lastCodePoint() + 1)
{
    m_numberToName.push_back("Cn");
    m_nameToNumber["Cn"] = 0;

    uint32_t last = m_unicodeData.lastCodePoint();
    string name;

    for (uint32_t codePoint = 0; codePoint <= last; codePoint++) {
        m_unicodeData.getGeneralCategory(codePoint, name);
        uint8_t number;

        if (name.length()) {
            auto match = m_nameToNumber.find(name);
            if (match != m_nameToNumber.end()) {
                number = match->second;
            } else {
                number = m_numberToName.size();
                m_numberToName.push_back(name);
                m_nameToNumber[name] = number;
            }
        } else {
            number = 0;
        }

        m_numbers[codePoint] = number;
    }
}

uint8_t GeneralCategoryDetector::numberForCodePoint(uint32_t codePoint) const {
    if (codePoint <= m_unicodeData.lastCodePoint()) {
        return m_numbers[codePoint];
    }

    return 0;
}

const string &GeneralCategoryDetector::nameForCodePoint(uint32_t codePoint) const {
    return m_numberToName[numberForCodePoint(codePoint)];
}

const string &GeneralCategoryDetector::numberToName(uint8_t number) const {
    if (number < m_numberToName.size()) {
        return m_numberToName[number];
    }

    return m_numberToName[0];
}

const uint8_t GeneralCategoryDetector::nameToNumber(const string &name) const {
    auto match = m_nameToNumber.find(name);
    if (match != m_nameToNumber.end()) {
        return match->second;
    }

    return 0;
}

void GeneralCategoryDetector::getAllNames(set<string> &names) const {
    for (auto begin = m_numberToName.begin(); begin < m_numberToName.end(); begin++) {
        names.insert(*begin);
    }
}
