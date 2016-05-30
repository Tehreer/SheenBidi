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

#include <cstdint>
#include <iostream>

extern "C" {
#include <SBBase.h>
#include <SBConfig.h>
#include <Source/SBPairingLookup.h>
}

#include <Parser/BidiMirroring.h>

#include "Utilities/Unicode.h"

#include "Configuration.h"
#include "MirrorTester.h"

using namespace std;
using namespace SheenBidi::Parser;
using namespace SheenBidi::Tester;
using namespace SheenBidi::Tester::Utilities;

MirrorTester::MirrorTester(const BidiMirroring &bidiMirroring) :
    m_BidiMirroring(bidiMirroring)
{
}

void MirrorTester::test() {
#ifdef SB_CONFIG_UNITY
    cout << "Cannot run mirror tester in unity mode." << endl;
#else
    cout << "Running mirror tester." << endl;

    size_t failCounter = 0;

    for (uint32_t codepoint = 0; codepoint < Unicode::MAX_CODE_POINT; codepoint++) {
        uint32_t expMirror = m_BidiMirroring.mirrorForCodePoint(codepoint);
        SBUInt32 genMirror = SBPairingDetermineMirror(codepoint);

        if (expMirror != genMirror) {
            if (Configuration::DISPLAY_ERROR_DETAILS) {
                cout << "Invalid mirror found: " << endl
                     << "  Code Point: " << codepoint << endl
                     << "  Generated Mirror: " << genMirror << endl
                     << "  Expected Mirror: " << expMirror << endl;
            }

            failCounter++;
        }
    }

    cout << failCounter << " error/s." << endl;
#endif

    cout << endl;
}
