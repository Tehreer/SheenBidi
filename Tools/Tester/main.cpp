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

extern "C" {
#include <SheenBidi.h>
}

#include <Parser/UnicodeData.h>
#include <Parser/BidiMirroring.h>
#include <Parser/BidiBrackets.h>
#include <Parser/BidiTest.h>
#include <Parser/BidiCharacterTest.h>

#include "CharTypeTester.h"
#include "CodepointSequenceTester.h"
#include "MirrorTester.h"
#include "BracketTester.h"
#include "AlgorithmTester.h"

using namespace std;
using namespace SheenBidi::Parser;
using namespace SheenBidi::Tester;

int main(int argc, const char *argv[]) {
    const char *dir = argv[1];

    UnicodeData unicodeData(dir);
    BidiMirroring bidiMirroring(dir);
    BidiBrackets bidiBrackets(dir);
    BidiTest bidiTest(dir);
    BidiCharacterTest bidiCharacterTest(dir);

    CharTypeTester charTypeTester(unicodeData);
    CodepointSequenceTester codepointSequenceTester;
    MirrorTester mirrorTester(bidiMirroring);
    BracketTester bracketTester(bidiBrackets);
    AlgorithmTester algorithmTester(&bidiTest, &bidiCharacterTest, &bidiMirroring);

    charTypeTester.test();
    codepointSequenceTester.test();
    mirrorTester.test();
    bracketTester.test();
    algorithmTester.test();

    return 0;
}
