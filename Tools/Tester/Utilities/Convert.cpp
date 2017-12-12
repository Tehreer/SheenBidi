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

#include <cstdint>
#include <map>
#include <string>

extern "C" {
#include <SBBase.h>
}

#include "Convert.h"

using namespace std;
using namespace SheenBidi::Tester::Utilities;

static map<SBBidiType, string> createStringMap() {
    map<SBBidiType, string> map;
    map[SBBidiTypeNil] = "Nil";
    map[SBBidiTypeL]   = "L";
    map[SBBidiTypeR]   = "R";
    map[SBBidiTypeAL]  = "AL";
    map[SBBidiTypeEN]  = "EN";
    map[SBBidiTypeES]  = "ES";
    map[SBBidiTypeET]  = "ET";
    map[SBBidiTypeAN]  = "AN";
    map[SBBidiTypeCS]  = "CS";
    map[SBBidiTypeNSM] = "NSM";
    map[SBBidiTypeBN]  = "BN";
    map[SBBidiTypeB]   = "B";
    map[SBBidiTypeS]   = "S";
    map[SBBidiTypeWS]  = "WS";
    map[SBBidiTypeON]  = "ON";
    map[SBBidiTypeLRE] = "LRE";
    map[SBBidiTypeRLE] = "RLE";
    map[SBBidiTypeLRO] = "LRO";
    map[SBBidiTypeRLO] = "RLO";
    map[SBBidiTypePDF] = "PDF";
    map[SBBidiTypeLRI] = "LRI";
    map[SBBidiTypeRLI] = "RLI";
    map[SBBidiTypeFSI] = "FSI";
    map[SBBidiTypePDI] = "PDI";

    return map;
}

static map<string, uint32_t> createCodePointMap() {
    map<string, uint32_t> map;
    map["AL"]  = 0x0627;
    map["AN"]  = 0x0600;
    map["B"]   = 0x0085;
    map["BN"]  = 0x001B;
    map["CS"]  = 0x002E;
    map["EN"]  = 0x0030;
    map["ES"]  = 0x002B;
    map["ET"]  = 0x0025;
    map["FSI"] = 0x2068;
    map["L"]   = 0x0041;
    map["LRE"] = 0x202A;
    map["LRI"] = 0x2066;
    map["LRO"] = 0x202D;
    map["NSM"] = 0x0614;
    map["ON"]  = 0x0028;
    map["PDF"] = 0x202C;
    map["PDI"] = 0x2069;
    map["R"]   = 0x05D0;
    map["RLE"] = 0x202B;
    map["RLI"] = 0x2067;
    map["RLO"] = 0x202E;
    map["S"]   = 0x0009;
    map["WS"]  = 0x0020;

    return map;
}

static map<SBBidiType, string> MAP_BIDI_TYPE_TO_STRING = createStringMap();
static map<string, uint32_t> MAP_BIDI_TYPE_TO_CODE_POINT = createCodePointMap();

const string &Convert::toString(SBBidiType bidiType) {
    return MAP_BIDI_TYPE_TO_STRING[bidiType];
}

uint32_t Convert::toCodePoint(const string &bidiType) {
    return MAP_BIDI_TYPE_TO_CODE_POINT[bidiType];
}
