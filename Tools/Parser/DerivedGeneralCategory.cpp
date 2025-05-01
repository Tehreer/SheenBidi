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
#include <cstddef>
#include <cstdint>
#include <string>

#include "DataFile.h"
#include "UnicodeVersion.h"
#include "DerivedGeneralCategory.h"

using namespace std;
using namespace SheenBidi::Parser;

static const string FILE_DERIVED_GENERAL_CATEGORY = "DerivedGeneralCategory.txt";
static const string GENERAL_CATEGORY_DEFAULT = "Cn";

DerivedGeneralCategory::DerivedGeneralCategory(const string &directory) :
    DataFile(directory, FILE_DERIVED_GENERAL_CATEGORY),
    m_generalCategories(MAX_CODE_POINTS)
{
    // Insert the default GeneralCategoryID for all code points.
    insertGeneralCategory(GENERAL_CATEGORY_DEFAULT);

    string line;
    if (readLine(line)) {
        getVersion(line, m_version);
    }

    while (readLine(line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        uint32_t firstCodePoint = 0;
        uint32_t lastCodePoint = 0;
        string generalCategory;

        size_t index = 0;
        index = getCodePointRange(line, index, firstCodePoint, lastCodePoint);
        getField(line, index, FieldTerminator::Space, generalCategory);

        auto id = insertGeneralCategory(generalCategory);
        for (uint32_t codePoint = firstCodePoint; codePoint <= lastCodePoint; codePoint++) {
            m_generalCategories.at(codePoint) = id;
        }

        m_lastCodePoint = max(m_lastCodePoint, lastCodePoint);
    }
}

DerivedGeneralCategory::GeneralCategoryID DerivedGeneralCategory::insertGeneralCategory(const string &generalCategory) {
    auto it = m_generalCategoryToID.find(generalCategory);
    if (it != m_generalCategoryToID.end()) {
        return it->second;
    }

    GeneralCategoryID id = static_cast<GeneralCategoryID>(m_idToGeneralCategory.size());
    m_generalCategoryToID[generalCategory] = id;
    m_idToGeneralCategory.push_back(generalCategory);

    return id;
}

const string &DerivedGeneralCategory::generalCategoryOf(uint32_t codePoint) const {
    return m_idToGeneralCategory.at(m_generalCategories.at(codePoint));
}
