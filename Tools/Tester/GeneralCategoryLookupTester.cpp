/*
 * Copyright (C) 2018-2019 Muhammad Tayyab Akram
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

extern "C" {
#include <Headers/SBBase.h>
#include <Headers/SBConfig.h>
#include <Headers/SBGeneralCategory.h>
#include <Source/GeneralCategoryLookup.h>
}

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>

#include <Parser/UnicodeData.h>

#include "Utilities/Convert.h"
#include "Utilities/Unicode.h"

#include "Configuration.h"
#include "GeneralCategoryLookupTester.h"

using namespace std;
using namespace SheenBidi::Parser;
using namespace SheenBidi::Tester;
using namespace SheenBidi::Tester::Utilities;

static const string DEFAULT_GENERAL_CATEGORY = "Cn";

GeneralCategoryLookupTester::GeneralCategoryLookupTester(const UnicodeData &unicodeData) :
    m_unicodeData(unicodeData)
{
}

void GeneralCategoryLookupTester::test()
{
#ifdef SB_CONFIG_UNITY
    cout << "Cannot run general category lookup tester in unity mode." << endl;
#else
    cout << "Running general category lookup tester." << endl;

    size_t failCounter = 0;
    string uniGeneralCategory;

    for (uint32_t codePoint = 0; codePoint <= Unicode::MAX_CODE_POINT; codePoint++) {
        m_unicodeData.getGeneralCategory(codePoint, uniGeneralCategory);
        const string &expGeneralCategory = (uniGeneralCategory.length() ? uniGeneralCategory : DEFAULT_GENERAL_CATEGORY);

        SBGeneralCategory valGeneralCategory = LookupGeneralCategory(codePoint);
        const string &genGeneralCategory = Convert::generalCategoryToString(valGeneralCategory);

        if (genGeneralCategory != expGeneralCategory) {
            if (Configuration::DISPLAY_ERROR_DETAILS) {
                cout << "Invalid general category found: " << endl
                     << "  Code Point: " << codePoint << endl
                     << "  Expected General Category: " << expGeneralCategory << endl
                     << "  Generated General Category: " << genGeneralCategory << endl;
            }

            failCounter++;
        }
    }

    cout << failCounter << " error/s." << endl;
    cout << endl;
#endif
}
