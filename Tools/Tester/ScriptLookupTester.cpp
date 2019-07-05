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
#include <Source/ScriptLookup.h>
}

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>

#include <Parser/PropertyValueAliases.h>
#include <Parser/Scripts.h>

#include "Utilities/Convert.h"
#include "Utilities/Unicode.h"

#include "Configuration.h"
#include "ScriptLookupTester.h"

using namespace std;
using namespace SheenBidi::Parser;
using namespace SheenBidi::Tester;
using namespace SheenBidi::Tester::Utilities;

ScriptLookupTester::ScriptLookupTester(const Scripts &scripts, const PropertyValueAliases &propertyValueAliases) :
    m_scripts(scripts),
    m_propertyValueAliases(propertyValueAliases)
{
}

void ScriptLookupTester::test() {
#ifdef SB_CONFIG_UNITY
    cout << "Cannot run script lookup tester in unity mode." << endl;
#else
    cout << "Running script lookup tester." << endl;

    size_t failCounter = 0;

    for (uint32_t codePoint = 0; codePoint <= Unicode::MAX_CODE_POINT; codePoint++) {
        const string &uniScript = m_scripts.scriptForCodePoint(codePoint);
        const string &expScript = m_propertyValueAliases.abbreviationForScript(uniScript);

        SBScript valScript = LookupScript(codePoint);
        const string &genScript = Convert::scriptToString(valScript);

        if (expScript != genScript) {
            if (Configuration::DISPLAY_ERROR_DETAILS) {
                cout << "Invalid script found: " << endl
                     << "  Code Point: " << codePoint << endl
                     << "  Expected Script: " << expScript << endl
                     << "  Generated Script: " << genScript << endl;
            }
                
            failCounter++;
        }
    }

    cout << failCounter << " error/s." << endl;
    cout << endl;
#endif
}
