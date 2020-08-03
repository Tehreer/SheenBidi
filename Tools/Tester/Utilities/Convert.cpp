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
#include <Headers/SBBidiType.h>
#include <Headers/SBGeneralCategory.h>
#include <Headers/SBScript.h>
}

#include <cstdint>
#include <map>
#include <string>

#include "Convert.h"

using namespace std;
using namespace SheenBidi::Tester::Utilities;

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

static map<string, uint32_t> MAP_BIDI_TYPE_TO_CODE_POINT = createCodePointMap();

static map<SBBidiType, string> MAP_BIDI_TYPE_TO_STRING = {
    {SBBidiTypeNil, "Nil"},
    {SBBidiTypeL, "L"},
    {SBBidiTypeR, "R"},
    {SBBidiTypeAL, "AL"},
    {SBBidiTypeEN, "EN"},
    {SBBidiTypeES, "ES"},
    {SBBidiTypeET, "ET"},
    {SBBidiTypeAN, "AN"},
    {SBBidiTypeCS, "CS"},
    {SBBidiTypeNSM, "NSM"},
    {SBBidiTypeBN, "BN"},
    {SBBidiTypeB, "B"},
    {SBBidiTypeS, "S"},
    {SBBidiTypeWS, "WS"},
    {SBBidiTypeON, "ON"},
    {SBBidiTypeLRE, "LRE"},
    {SBBidiTypeRLE, "RLE"},
    {SBBidiTypeLRO, "LRO"},
    {SBBidiTypeRLO, "RLO"},
    {SBBidiTypePDF, "PDF"},
    {SBBidiTypeLRI, "LRI"},
    {SBBidiTypeRLI, "RLI"},
    {SBBidiTypeFSI, "FSI"},
    {SBBidiTypePDI, "PDI"}
};

static map<SBGeneralCategory, string> MAP_GENERAL_CATEGORY_TO_STRING = {
    {SBGeneralCategoryLU, "Lu"},
    {SBGeneralCategoryLL, "Ll"},
    {SBGeneralCategoryLT, "Lt"},
    {SBGeneralCategoryLM, "Lm"},
    {SBGeneralCategoryLO, "Lo"},
    {SBGeneralCategoryMN, "Mn"},
    {SBGeneralCategoryMC, "Mc"},
    {SBGeneralCategoryME, "Me"},
    {SBGeneralCategoryND, "Nd"},
    {SBGeneralCategoryNL, "Nl"},
    {SBGeneralCategoryNO, "No"},
    {SBGeneralCategoryPC, "Pc"},
    {SBGeneralCategoryPD, "Pd"},
    {SBGeneralCategoryPS, "Ps"},
    {SBGeneralCategoryPE, "Pe"},
    {SBGeneralCategoryPI, "Pi"},
    {SBGeneralCategoryPF, "Pf"},
    {SBGeneralCategoryPO, "Po"},
    {SBGeneralCategorySM, "Sm"},
    {SBGeneralCategorySC, "Sc"},
    {SBGeneralCategorySK, "Sk"},
    {SBGeneralCategorySO, "So"},
    {SBGeneralCategoryZS, "Zs"},
    {SBGeneralCategoryZL, "Zl"},
    {SBGeneralCategoryZP, "Zp"},
    {SBGeneralCategoryCC, "Cc"},
    {SBGeneralCategoryCF, "Cf"},
    {SBGeneralCategoryCS, "Cs"},
    {SBGeneralCategoryCO, "Co"},
    {SBGeneralCategoryCN, "Cn"}
};

static map<SBScript, string> MAP_SCRIPT_TO_STRING = {
    {SBScriptZINH, "Zinh"},
    {SBScriptZYYY, "Zyyy"},
    {SBScriptZZZZ, "Zzzz"},
    {SBScriptARAB, "Arab"},
    {SBScriptARMN, "Armn"},
    {SBScriptBENG, "Beng"},
    {SBScriptBOPO, "Bopo"},
    {SBScriptCYRL, "Cyrl"},
    {SBScriptDEVA, "Deva"},
    {SBScriptGEOR, "Geor"},
    {SBScriptGREK, "Grek"},
    {SBScriptGUJR, "Gujr"},
    {SBScriptGURU, "Guru"},
    {SBScriptHANG, "Hang"},
    {SBScriptHANI, "Hani"},
    {SBScriptHEBR, "Hebr"},
    {SBScriptHIRA, "Hira"},
    {SBScriptKANA, "Kana"},
    {SBScriptKNDA, "Knda"},
    {SBScriptLAOO, "Laoo"},
    {SBScriptLATN, "Latn"},
    {SBScriptMLYM, "Mlym"},
    {SBScriptORYA, "Orya"},
    {SBScriptTAML, "Taml"},
    {SBScriptTELU, "Telu"},
    {SBScriptTHAI, "Thai"},
    {SBScriptTIBT, "Tibt"},
    {SBScriptBRAI, "Brai"},
    {SBScriptCANS, "Cans"},
    {SBScriptCHER, "Cher"},
    {SBScriptETHI, "Ethi"},
    {SBScriptKHMR, "Khmr"},
    {SBScriptMONG, "Mong"},
    {SBScriptMYMR, "Mymr"},
    {SBScriptOGAM, "Ogam"},
    {SBScriptRUNR, "Runr"},
    {SBScriptSINH, "Sinh"},
    {SBScriptSYRC, "Syrc"},
    {SBScriptTHAA, "Thaa"},
    {SBScriptYIII, "Yiii"},
    {SBScriptDSRT, "Dsrt"},
    {SBScriptGOTH, "Goth"},
    {SBScriptITAL, "Ital"},
    {SBScriptBUHD, "Buhd"},
    {SBScriptHANO, "Hano"},
    {SBScriptTAGB, "Tagb"},
    {SBScriptTGLG, "Tglg"},
    {SBScriptCPRT, "Cprt"},
    {SBScriptLIMB, "Limb"},
    {SBScriptLINB, "Linb"},
    {SBScriptOSMA, "Osma"},
    {SBScriptSHAW, "Shaw"},
    {SBScriptTALE, "Tale"},
    {SBScriptUGAR, "Ugar"},
    {SBScriptBUGI, "Bugi"},
    {SBScriptCOPT, "Copt"},
    {SBScriptGLAG, "Glag"},
    {SBScriptKHAR, "Khar"},
    {SBScriptSYLO, "Sylo"},
    {SBScriptTALU, "Talu"},
    {SBScriptTFNG, "Tfng"},
    {SBScriptXPEO, "Xpeo"},
    {SBScriptBALI, "Bali"},
    {SBScriptNKOO, "Nkoo"},
    {SBScriptPHAG, "Phag"},
    {SBScriptPHNX, "Phnx"},
    {SBScriptXSUX, "Xsux"},
    {SBScriptCARI, "Cari"},
    {SBScriptCHAM, "Cham"},
    {SBScriptKALI, "Kali"},
    {SBScriptLEPC, "Lepc"},
    {SBScriptLYCI, "Lyci"},
    {SBScriptLYDI, "Lydi"},
    {SBScriptOLCK, "Olck"},
    {SBScriptRJNG, "Rjng"},
    {SBScriptSAUR, "Saur"},
    {SBScriptSUND, "Sund"},
    {SBScriptVAII, "Vaii"},
    {SBScriptARMI, "Armi"},
    {SBScriptAVST, "Avst"},
    {SBScriptBAMU, "Bamu"},
    {SBScriptEGYP, "Egyp"},
    {SBScriptJAVA, "Java"},
    {SBScriptKTHI, "Kthi"},
    {SBScriptLANA, "Lana"},
    {SBScriptLISU, "Lisu"},
    {SBScriptMTEI, "Mtei"},
    {SBScriptORKH, "Orkh"},
    {SBScriptPHLI, "Phli"},
    {SBScriptPRTI, "Prti"},
    {SBScriptSAMR, "Samr"},
    {SBScriptSARB, "Sarb"},
    {SBScriptTAVT, "Tavt"},
    {SBScriptBATK, "Batk"},
    {SBScriptBRAH, "Brah"},
    {SBScriptMAND, "Mand"},
    {SBScriptCAKM, "Cakm"},
    {SBScriptMERC, "Merc"},
    {SBScriptMERO, "Mero"},
    {SBScriptPLRD, "Plrd"},
    {SBScriptSHRD, "Shrd"},
    {SBScriptSORA, "Sora"},
    {SBScriptTAKR, "Takr"},
    {SBScriptAGHB, "Aghb"},
    {SBScriptBASS, "Bass"},
    {SBScriptDUPL, "Dupl"},
    {SBScriptELBA, "Elba"},
    {SBScriptGRAN, "Gran"},
    {SBScriptHMNG, "Hmng"},
    {SBScriptKHOJ, "Khoj"},
    {SBScriptLINA, "Lina"},
    {SBScriptMAHJ, "Mahj"},
    {SBScriptMANI, "Mani"},
    {SBScriptMEND, "Mend"},
    {SBScriptMODI, "Modi"},
    {SBScriptMROO, "Mroo"},
    {SBScriptNARB, "Narb"},
    {SBScriptNBAT, "Nbat"},
    {SBScriptPALM, "Palm"},
    {SBScriptPAUC, "Pauc"},
    {SBScriptPERM, "Perm"},
    {SBScriptPHLP, "Phlp"},
    {SBScriptSIDD, "Sidd"},
    {SBScriptSIND, "Sind"},
    {SBScriptTIRH, "Tirh"},
    {SBScriptWARA, "Wara"},
    {SBScriptAHOM, "Ahom"},
    {SBScriptHATR, "Hatr"},
    {SBScriptHLUW, "Hluw"},
    {SBScriptHUNG, "Hung"},
    {SBScriptMULT, "Mult"},
    {SBScriptSGNW, "Sgnw"},
    {SBScriptADLM, "Adlm"},
    {SBScriptBHKS, "Bhks"},
    {SBScriptMARC, "Marc"},
    {SBScriptNEWA, "Newa"},
    {SBScriptOSGE, "Osge"},
    {SBScriptTANG, "Tang"},
    {SBScriptGONM, "Gonm"},
    {SBScriptNSHU, "Nshu"},
    {SBScriptSOYO, "Soyo"},
    {SBScriptZANB, "Zanb"},
    {SBScriptDOGR, "Dogr"},
    {SBScriptGONG, "Gong"},
    {SBScriptMAKA, "Maka"},
    {SBScriptMEDF, "Medf"},
    {SBScriptROHG, "Rohg"},
    {SBScriptSOGD, "Sogd"},
    {SBScriptSOGO, "Sogo"},
    {SBScriptELYM, "Elym"},
    {SBScriptHMNP, "Hmnp"},
    {SBScriptNAND, "Nand"},
    {SBScriptWCHO, "Wcho"},
    {SBScriptCHRS, "Chrs"},
    {SBScriptDIAK, "Diak"},
    {SBScriptKITS, "Kits"},
    {SBScriptYEZI, "Yezi"},
};

const string &Convert::bidiTypeToString(SBBidiType bidiType) {
    return MAP_BIDI_TYPE_TO_STRING[bidiType];
}

const string &Convert::generalCategoryToString(SBGeneralCategory generalCategory) {
    return MAP_GENERAL_CATEGORY_TO_STRING[generalCategory];
}

const string &Convert::scriptToString(SBScript script) {
    return MAP_SCRIPT_TO_STRING[script];
}

uint32_t Convert::toCodePoint(const string &bidiType) {
    return MAP_BIDI_TYPE_TO_CODE_POINT[bidiType];
}
