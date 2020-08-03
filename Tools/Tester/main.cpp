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
#include <Headers/SheenBidi.h>
}

#include <iostream>

#include <Parser/BidiBrackets.h>
#include <Parser/BidiCharacterTest.h>
#include <Parser/BidiMirroring.h>
#include <Parser/BidiTest.h>
#include <Parser/DerivedBidiClass.h>
#include <Parser/PropertyValueAliases.h>
#include <Parser/Scripts.h>
#include <Parser/UnicodeData.h>

#include "AlgorithmTester.h"
#include "BidiTypeLookupTester.h"
#include "BracketLookupTester.h"
#include "CodepointSequenceTester.h"
#include "GeneralCategoryLookupTester.h"
#include "MirrorLookupTester.h"
#include "ScriptLocatorTester.h"
#include "ScriptLookupTester.h"

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
    DerivedBidiClass derivedBidiClass(dir);
    Scripts scripts(dir);
    PropertyValueAliases propertyValueAliases(dir);

    BidiTypeLookupTester bidiTypeLookupTester(derivedBidiClass);
    CodepointSequenceTester codepointSequenceTester;
    MirrorLookupTester mirrorLookupTester(bidiMirroring);
    BracketLookupTester bracketLookupTester(bidiBrackets);
    GeneralCategoryLookupTester generalCategoryLookupTester(unicodeData);
    ScriptLookupTester scriptLookupTester(scripts, propertyValueAliases);
    AlgorithmTester algorithmTester(&bidiTest, &bidiCharacterTest, &bidiMirroring);
    ScriptLocatorTester scriptLocatorTester;

    bidiTypeLookupTester.test();
    codepointSequenceTester.test();
    mirrorLookupTester.test();
    bracketLookupTester.test();
    generalCategoryLookupTester.test();
    scriptLookupTester.test();
    algorithmTester.test();
    scriptLocatorTester.test();

    return 0;
}
