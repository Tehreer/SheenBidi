/*
 * Copyright (C) 2015-2019 Muhammad Tayyab Akram
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
#include <Source/PairingLookup.h>
}

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>

#include <Parser/BidiMirroring.h>

#include "Utilities/Unicode.h"

#include "Configuration.h"
#include "MirrorLookupTester.h"

using namespace std;
using namespace SheenBidi::Parser;
using namespace SheenBidi::Tester;
using namespace SheenBidi::Tester::Utilities;

MirrorLookupTester::MirrorLookupTester(const BidiMirroring &bidiMirroring) :
    m_BidiMirroring(bidiMirroring)
{
}

void MirrorLookupTester::test() {
#ifdef SB_CONFIG_UNITY
    cout << "Cannot run mirror lookup tester in unity mode." << endl;
#else
    cout << "Running mirror lookup tester." << endl;

    size_t failCounter = 0;

    for (uint32_t codePoint = 0; codePoint <= Unicode::MAX_CODE_POINT; codePoint++) {
        uint32_t expMirror = m_BidiMirroring.mirrorForCodePoint(codePoint);
        SBUInt32 genMirror = LookupMirror(codePoint);

        if (genMirror != expMirror) {
            if (Configuration::DISPLAY_ERROR_DETAILS) {
                cout << "Invalid mirror found: " << endl
                     << "  Code Point: " << codePoint << endl
                     << "  Generated Mirror: " << genMirror << endl
                     << "  Expected Mirror: " << expMirror << endl;
            }

            failCounter++;
        }
    }

    cout << failCounter << " error/s." << endl;
    cout << endl;
#endif
}
