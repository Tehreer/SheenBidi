/*
 * Copyright (C) 2015-2020 Muhammad Tayyab Akram
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
#include <Headers/SBBidiType.h>
#include <Headers/SBConfig.h>
#include <Source/BidiTypeLookup.h>
}

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>

#include <Parser/DerivedBidiClass.h>

#include "Utilities/Convert.h"
#include "Utilities/Unicode.h"

#include "Configuration.h"
#include "BidiTypeLookupTester.h"

using namespace std;
using namespace SheenBidi::Parser;
using namespace SheenBidi::Tester;
using namespace SheenBidi::Tester::Utilities;

static const string BIDI_TYPE_DEFAULT = "ON";

BidiTypeLookupTester::BidiTypeLookupTester(const DerivedBidiClass &derivedBidiClass) :
    m_derivedBidiClass(derivedBidiClass)
{
}

void BidiTypeLookupTester::test() {
#ifdef SB_CONFIG_UNITY
    cout << "Cannot run bidi type lookup tester in unity mode." << endl;
#else
    cout << "Running bidi type lookup tester." << endl;

    size_t failCounter = 0;

    for (uint32_t codePoint = 0; codePoint <= Unicode::MAX_CODE_POINT; codePoint++) {
        const string &uniBidiType = m_derivedBidiClass.bidiClassForCodePoint(codePoint);
        const string &expBidiType = (uniBidiType.length() ? uniBidiType : BIDI_TYPE_DEFAULT);

        SBBidiType valBidiType = LookupBidiType(codePoint);
        const string &genBidiType = Convert::bidiTypeToString(valBidiType);

        if (genBidiType != expBidiType) {
            if (Configuration::DISPLAY_ERROR_DETAILS) {
                cout << "Invalid char type found: " << endl
                     << "  Code Point: " << codePoint << endl
                     << "  Expected Char Type: " << expBidiType << endl
                     << "  Generated Char Type: " << genBidiType << endl;
            }

            failCounter++;
        }
    }

    cout << failCounter << " error/s." << endl;
    cout << endl;
#endif
}
