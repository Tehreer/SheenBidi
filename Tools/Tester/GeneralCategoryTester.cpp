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

extern "C" {
#include <SBBase.h>
#include <SBConfig.h>
#include <Source/SBGeneralCategoryLookup.h>
}

#include <cassert>
#include <cstdint>
#include <iostream>

#include <Parser/UnicodeData.h>

#include "Utilities/Convert.h"
#include "Utilities/Unicode.h"

#include "Configuration.h"
#include "GeneralCategoryTester.h"

using namespace std;
using namespace SheenBidi::Parser;
using namespace SheenBidi::Tester;
using namespace SheenBidi::Tester::Utilities;

static const string &DEFAULT_GENERAL_CATEGORY = "Cn";

GeneralCategoryTester::GeneralCategoryTester(const UnicodeData &unicodeData) :
    m_unicodeData(unicodeData)
{
}

void GeneralCategoryTester::test()
{
#ifdef SB_CONFIG_UNITY
    cout << "Cannot run general category tester in unity mode." << endl;
#else
    cout << "Running general category tester." << endl;

    size_t failCounter = 0;
    string uniGeneralCategory;

    for (uint32_t codepoint = 0; codepoint < Unicode::MAX_CODE_POINT; codepoint++) {
        m_unicodeData.getGeneralCategory(codepoint, uniGeneralCategory);
        const string &expected = (uniGeneralCategory.length() ? uniGeneralCategory : DEFAULT_GENERAL_CATEGORY);

        SBGeneralCategory libGeneralCategory = SBGeneralCategoryDetermine(codepoint);
        const string &actual = Convert::generalCategoryToString(libGeneralCategory);

        if (actual != expected) {
            if (Configuration::DISPLAY_ERROR_DETAILS) {
                cout << "Invalid general category found: " << endl
                     << "  Code Point: " << codepoint << endl
                     << "  Expected General Category: " << expected << endl
                     << "  Generated General Category: " << actual << endl;
            }

            failCounter++;
        }
    }

    cout << failCounter << " error/s." << endl;
    cout << endl;
#endif
}
