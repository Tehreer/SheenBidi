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

#include <cstdint>
#include <string>
#include <vector>

#include "DataFile.h"
#include "DerivedCoreProperties.h"

using namespace std;
using namespace SheenBidi::Parser;

static const string FILE_DERIVED_CORE_PROPERTIES = "DerivedCoreProperties.txt";
static const string PROPERTY_DEFAULT_IGNORABLE = "Default_Ignorable_Code_Point";

DerivedCoreProperties::DerivedCoreProperties(const string &directory) :
    DataFile(directory, FILE_DERIVED_CORE_PROPERTIES),
    m_codePointProperties(CodePointCount)
{
    Line line;
    string propertyName;

    if (readLine(line)) {
        m_version = line.scanVersion();
    }

    while (readLine(line)) {
        if (line.isEmpty() || line.match('#')) {
            continue;
        }

        auto range = line.parseCodePointRange();
        line.getField(propertyName);

        auto propertyBit = insertProperty(propertyName);
        for (auto codePoint = range.first; codePoint <= range.second; codePoint++) {
            m_codePointProperties.at(codePoint).set(propertyBit);
        }
    }

    m_defaultIgnorableBit = m_propertyToBit.at(PROPERTY_DEFAULT_IGNORABLE);
}

DerivedCoreProperties::PropertyBit DerivedCoreProperties::insertProperty(const string &property) {
    return m_propertyToBit.insert({property, m_propertyToBit.size()}).first->second;
}

vector<string> DerivedCoreProperties::getProperties(uint32_t codePoint) const {
    vector<string> result;

    const auto &bits = m_codePointProperties.at(codePoint);
    for (const auto &kv : m_propertyToBit) {
        if (bits.test(kv.second)) {
            result.push_back(kv.first);
        }
    }

    return result;
}

bool DerivedCoreProperties::hasProperty(uint32_t codePoint, const string &property) const {
    const auto &bits = m_codePointProperties.at(codePoint);

    auto it = m_propertyToBit.find(property);
    if (it == m_propertyToBit.end()) {
        return false;
    }

    return bits.test(it->second);
}

bool DerivedCoreProperties::isDefaultIgnorable(uint32_t codePoint) const {
    return m_codePointProperties.at(codePoint).test(m_defaultIgnorableBit);
}
