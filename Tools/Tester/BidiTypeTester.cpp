/*
 * Copyright (C) 2017 Muhammad Tayyab Akram
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
#include <Source/SBBidiTypeLookup.h>
}

#include <cstdint>
#include <iostream>
#include <string>

#include <Parser/UnicodeData.h>

#include "Utilities/Convert.h"
#include "Utilities/Unicode.h"

#include "Configuration.h"
#include "BidiTypeTester.h"

using namespace std;
using namespace SheenBidi::Parser;
using namespace SheenBidi::Tester;
using namespace SheenBidi::Tester::Utilities;

static const string &BIDI_CLASS_DEFAULT = "L";

BidiTypeTester::BidiTypeTester(const UnicodeData &unicodeData)
    : m_unicodeData(unicodeData)
{
}

void BidiTypeTester::test() {
#ifdef SB_CONFIG_UNITY
    cout << "Cannot run char type tester in unity mode." << endl;
#else
    cout << "Running char type tester." << endl;

    size_t failCounter = 0;

    for (uint32_t codePoint = 0; codePoint <= Unicode::MAX_CODE_POINT; codePoint++) {
        const string &uniClass = m_unicodeData.bidiClassForCodePoint(codePoint);
        const string &expClass = (uniClass.length() ? uniClass : BIDI_CLASS_DEFAULT);

        SBBidiType bidiType = SBBidiTypeDetermine(codePoint);
        const string &genClass = Convert::toString(bidiType);

        if (expClass != genClass) {
            if (Configuration::DISPLAY_ERROR_DETAILS) {
                cout << "Invalid char type found: " << endl
                     << "  Code Point: " << codePoint << endl
                     << "  Expected Char Type: " << expClass << endl
                     << "  Generated Char Type: " << genClass << endl;
            }
                
            failCounter++;
        }
    }

    cout << failCounter << " error/s." << endl;
#endif

    cout << endl;
}
