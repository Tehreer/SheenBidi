/*
 * Copyright (C) 2016 Muhammad Tayyab Akram
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

#include <iostream>

extern "C" {
#include <SBBase.h>
#include <SBConfig.h>
#include <Source/SBBracketType.h>
#include <Source/SBPairingLookup.h>
}

#include <Parser/BidiBrackets.h>

#include "Utilities/Unicode.h"

#include "Configuration.h"
#include "BracketTester.h"

using namespace std;
using namespace SheenBidi::Parser;
using namespace SheenBidi::Tester;
using namespace SheenBidi::Tester::Utilities;

BracketTester::BracketTester(const BidiBrackets &bidiBrackets)
    : m_BidiBrackets(bidiBrackets)
{
}

void BracketTester::test() {
#ifdef SB_CONFIG_UNITY
    cout << "Cannot run bracket tester in unity mode." << endl;
#else
    cout << "Running bracket tester." << endl;

    size_t failCounter = 0;

    for (uint32_t codePoint = 0; codePoint < Unicode::MAX_CODE_POINT; codePoint++) {
        uint32_t expBracket = m_BidiBrackets.pairedBracketForCodePoint(codePoint);
        char expType = m_BidiBrackets.pairedBracketTypeForCodePoint(codePoint);

        SBBracketType type;
        SBUInt32 genBracket = SBPairingDetermineBracketPair(codePoint, &type);

        char genType;
        switch (type) {
        case SBBracketTypeOpen:
            genType = 'o';
            break;

        case SBBracketTypeClose:
            genType = 'c';
            break;

        default:
            genType = '\0';
            break;
        }

        if (expBracket != genBracket || expType != genType) {
            if (Configuration::DISPLAY_ERROR_DETAILS) {
                cout << "Invalid bracket pair found: " << endl
                     << "  Code Point: " << codePoint << endl
                     << "  Expected Bracket: " << expBracket << endl
                     << "  Expected Type: " << expBracket << endl
                     << "  Generated Bracket: " << genBracket << endl
                     << "  Generated Type: " << genType << endl;
            }

            failCounter++;
        }
    }

    cout << failCounter << " error/s." << endl;
#endif

    cout << endl;
}
