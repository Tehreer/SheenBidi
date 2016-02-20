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
#include <map>
#include <string>

extern "C" {
#include <Source/SBCharType.h>
}

#include "Converter.h"

using namespace std;
using namespace SheenBidi::Tester::Utilities;

static map<SBCharType, string> createStringMap() {
    map<SBCharType, string> map;
    map[SBCharTypeNil] = "Nil";
    map[SBCharTypeL]   = "L";
    map[SBCharTypeR]   = "R";
    map[SBCharTypeAL]  = "AL";
    map[SBCharTypeEN]  = "EN";
    map[SBCharTypeES]  = "ES";
    map[SBCharTypeET]  = "ET";
    map[SBCharTypeAN]  = "AN";
    map[SBCharTypeCS]  = "CS";
    map[SBCharTypeNSM] = "NSM";
    map[SBCharTypeBN]  = "BN";
    map[SBCharTypeB]   = "B";
    map[SBCharTypeS]   = "S";
    map[SBCharTypeWS]  = "WS";
    map[SBCharTypeON]  = "ON";
    map[SBCharTypeLRE] = "LRE";
    map[SBCharTypeRLE] = "RLE";
    map[SBCharTypeLRO] = "LRO";
    map[SBCharTypeRLO] = "RLO";
    map[SBCharTypePDF] = "PDF";
    map[SBCharTypeLRI] = "LRI";
    map[SBCharTypeRLI] = "RLI";
    map[SBCharTypeFSI] = "FSI";
    map[SBCharTypePDI] = "PDI";

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

static map<SBCharType, string> MAP_CHAR_TYPE_TO_STRING = createStringMap();
static map<string, uint32_t> MAP_CHAR_TYPE_TO_CODE_POINT = createCodePointMap();

const string &Converter::toString(SBCharType charType) {
    return MAP_CHAR_TYPE_TO_STRING[charType];
}

uint32_t Converter::toCodePoint(const string &charType) {
    return MAP_CHAR_TYPE_TO_CODE_POINT[charType];
}
