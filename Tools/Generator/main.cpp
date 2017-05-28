/*
 * Copyright (C) 2015 Muhammad Tayyab Akram
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

#include <cstdio>
#include <iostream>
#include <string>

#include <Parser/UnicodeVersion.h>
#include <Parser/BidiMirroring.h>
#include <Parser/BidiBrackets.h>
#include <Parser/UnicodeData.h>
#include <Parser/BidiCharacterTest.h>

#include "PairingLookupGenerator.h"
#include "CharTypeLookupGenerator.h"

using namespace std;
using namespace SheenBidi::Parser;
using namespace SheenBidi::Generator;

int main(int argc, const char * argv[])
{
    const string in = "/path/to/input";
    const string out = "/path/to/output";

    UnicodeData unicodeData(in);
    BidiMirroring bidiMirroring(in);
    BidiBrackets bidiBrackets(in);

    cout << "Generating files." << endl;

    CharTypeLookupGenerator charTypeLookup(unicodeData);
    charTypeLookup.setMainSegmentSize(16);
    charTypeLookup.setBranchSegmentSize(100);
    charTypeLookup.generateFile(out);

    PairingLookupGenerator pairingLookup(bidiMirroring, bidiBrackets);
    pairingLookup.setSegmentSize(113);
    pairingLookup.generateFile(out);

    cout << "Finished.";

    getchar();

    return 0;
}
