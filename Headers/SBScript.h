/*
 * Copyright (C) 2018 Muhammad Tayyab Akram
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

#ifndef _SB_PUBLIC_SCRIPT_H
#define _SB_PUBLIC_SCRIPT_H

#include "SBBase.h"

/**
 * Constants that specify the script of a character.
 */
enum {
    SBScriptZINH = 0x00,    /**< Inherited */
    SBScriptZYYY = 0x01,    /**< Common */
    SBScriptZZZZ = 0x02,    /**< Unknown */

    /* Unicode 1.1  */
    SBScriptARAB = 0x03,    /**< Arabic */
    SBScriptARMN = 0x04,    /**< Armenian */
    SBScriptBENG = 0x05,    /**< Bengali */
    SBScriptBOPO = 0x06,    /**< Bopomofo */
    SBScriptCYRL = 0x07,    /**< Cyrillic */
    SBScriptDEVA = 0x08,    /**< Devanagari */
    SBScriptGEOR = 0x09,    /**< Georgian */
    SBScriptGREK = 0x0A,    /**< Greek */
    SBScriptGUJR = 0x0B,    /**< Gujarati */
    SBScriptGURU = 0x0C,    /**< Gurmukhi */
    SBScriptHANG = 0x0D,    /**< Hangul */
    SBScriptHANI = 0x0E,    /**< Han */
    SBScriptHEBR = 0x0F,    /**< Hebrew */
    SBScriptHIRA = 0x10,    /**< Hiragana */
    SBScriptKANA = 0x11,    /**< Katakana */
    SBScriptKNDA = 0x12,    /**< Kannada */
    SBScriptLAOO = 0x13,    /**< Lao */
    SBScriptLATN = 0x14,    /**< Latin */
    SBScriptMLYM = 0x15,    /**< Malayalam */
    SBScriptORYA = 0x16,    /**< Oriya */
    SBScriptTAML = 0x17,    /**< Tamil */
    SBScriptTELU = 0x18,    /**< Telugu */
    SBScriptTHAI = 0x19,    /**< Thai */

    /* Unicode 2.0 */
    SBScriptTIBT = 0x1A,    /**< Tibetan */

    /* Unicode 3.0 */
    SBScriptBRAI = 0x1B,    /**< Braille */
    SBScriptCANS = 0x1C,    /**< Canadian_Aboriginal */
    SBScriptCHER = 0x1D,    /**< Cherokee */
    SBScriptETHI = 0x1E,    /**< Ethiopic */
    SBScriptKHMR = 0x1F,    /**< Khmer */
    SBScriptMONG = 0x20,    /**< Mongolian */
    SBScriptMYMR = 0x21,    /**< Myanmar */
    SBScriptOGAM = 0x22,    /**< Ogham */
    SBScriptRUNR = 0x23,    /**< Runic */
    SBScriptSINH = 0x24,    /**< Sinhala */
    SBScriptSYRC = 0x25,    /**< Syriac */
    SBScriptTHAA = 0x26,    /**< Thaana */
    SBScriptYIII = 0x27,    /**< Yi */

    /* Unicode 3.1 */
    SBScriptDSRT = 0x28,    /**< Deseret */
    SBScriptGOTH = 0x29,    /**< Gothic */
    SBScriptITAL = 0x2A,    /**< Old_Italic */

    /* Unicode 3.2 */
    SBScriptBUHD = 0x2B,    /**< Buhid */
    SBScriptHANO = 0x2C,    /**< Hanunoo */
    SBScriptTAGB = 0x2D,    /**< Tagbanwa */
    SBScriptTGLG = 0x2E,    /**< Tagalog */

    /* Unicode 4.0 */
    SBScriptCPRT = 0x2F,    /**< Cypriot */
    SBScriptLIMB = 0x30,    /**< Limbu */
    SBScriptLINB = 0x31,    /**< Linear_B */
    SBScriptOSMA = 0x32,    /**< Osmanya */
    SBScriptSHAW = 0x33,    /**< Shavian */
    SBScriptTALE = 0x34,    /**< Tai_Le */
    SBScriptUGAR = 0x35,    /**< Ugaritic */

    /* Unicode 4.1 */
    SBScriptBUGI = 0x36,    /**< Buginese */
    SBScriptCOPT = 0x37,    /**< Coptic */
    SBScriptGLAG = 0x38,    /**< Glagolitic */
    SBScriptKHAR = 0x39,    /**< Kharoshthi */
    SBScriptSYLO = 0x3A,    /**< Syloti_Nagri */
    SBScriptTALU = 0x3B,    /**< New_Tai_Lue */
    SBScriptTFNG = 0x3C,    /**< Tifinagh */
    SBScriptXPEO = 0x3D,    /**< Old_Persian */

    /* Unicode 5.0 */
    SBScriptBALI = 0x3E,    /**< Balinese */
    SBScriptNKOO = 0x3F,    /**< Nko */
    SBScriptPHAG = 0x40,    /**< Phags_Pa */
    SBScriptPHNX = 0x41,    /**< Phoenician */
    SBScriptXSUX = 0x42,    /**< Cuneiform */

    /* Unicode 5.1 */
    SBScriptCARI = 0x43,    /**< Carian */
    SBScriptCHAM = 0x44,    /**< Cham */
    SBScriptKALI = 0x45,    /**< Kayah_Li */
    SBScriptLEPC = 0x46,    /**< Lepcha */
    SBScriptLYCI = 0x47,    /**< Lycian */
    SBScriptLYDI = 0x48,    /**< Lydian */
    SBScriptOLCK = 0x49,    /**< Ol_Chiki */
    SBScriptRJNG = 0x4A,    /**< Rejang */
    SBScriptSAUR = 0x4B,    /**< Saurashtra */
    SBScriptSUND = 0x4C,    /**< Sundanese */
    SBScriptVAII = 0x4D,    /**< Vai */

    /* Unicode 5.2 */
    SBScriptARMI = 0x4E,    /**< Imperial_Aramaic */
    SBScriptAVST = 0x4F,    /**< Avestan */
    SBScriptBAMU = 0x50,    /**< Bamum */
    SBScriptEGYP = 0x51,    /**< Egyptian_Hieroglyphs */
    SBScriptJAVA = 0x52,    /**< Javanese */
    SBScriptKTHI = 0x53,    /**< Kaithi */
    SBScriptLANA = 0x54,    /**< Tai_Tham */
    SBScriptLISU = 0x55,    /**< Lisu */
    SBScriptMTEI = 0x56,    /**< Meetei_Mayek */
    SBScriptORKH = 0x57,    /**< Old_Turkic */
    SBScriptPHLI = 0x58,    /**< Inscriptional_Pahlavi */
    SBScriptPRTI = 0x59,    /**< Inscriptional_Parthian */
    SBScriptSAMR = 0x5A,    /**< Samaritan */
    SBScriptSARB = 0x5B,    /**< Old_South_Arabian */
    SBScriptTAVT = 0x5C,    /**< Tai_Viet */

    /* Unicode 6.0 */
    SBScriptBATK = 0x5D,    /**< Batak */
    SBScriptBRAH = 0x5E,    /**< Brahmi */
    SBScriptMAND = 0x5F,    /**< Mandaic */

    /* Unicode 6.1 */
    SBScriptCAKM = 0x60,    /**< Chakma */
    SBScriptMERC = 0x61,    /**< Meroitic_Cursive */
    SBScriptMERO = 0x62,    /**< Meroitic_Hieroglyphs */
    SBScriptPLRD = 0x63,    /**< Miao */
    SBScriptSHRD = 0x64,    /**< Sharada */
    SBScriptSORA = 0x65,    /**< Sora_Sompeng */
    SBScriptTAKR = 0x66,    /**< Takri */

    /* Unicode 7.0 */
    SBScriptAGHB = 0x67,    /**< Caucasian_Albanian */
    SBScriptBASS = 0x68,    /**< Bassa_Vah */
    SBScriptDUPL = 0x69,    /**< Duployan */
    SBScriptELBA = 0x6A,    /**< Elbasan */
    SBScriptGRAN = 0x6B,    /**< Grantha */
    SBScriptHMNG = 0x6C,    /**< Pahawh_Hmong */
    SBScriptKHOJ = 0x6D,    /**< Khojki */
    SBScriptLINA = 0x6E,    /**< Linear_A */
    SBScriptMAHJ = 0x6F,    /**< Mahajani */
    SBScriptMANI = 0x70,    /**< Manichaean */
    SBScriptMEND = 0x71,    /**< Mende_Kikakui */
    SBScriptMODI = 0x72,    /**< Modi */
    SBScriptMROO = 0x73,    /**< Mro */
    SBScriptNARB = 0x74,    /**< Old_North_Arabian */
    SBScriptNBAT = 0x75,    /**< Nabataean */
    SBScriptPALM = 0x76,    /**< Palmyrene */
    SBScriptPAUC = 0x77,    /**< Pau_Cin_Hau */
    SBScriptPERM = 0x78,    /**< Old_Permic */
    SBScriptPHLP = 0x79,    /**< Psalter_Pahlavi */
    SBScriptSIDD = 0x7A,    /**< Siddham */
    SBScriptSIND = 0x7B,    /**< Khudawadi */
    SBScriptTIRH = 0x7C,    /**< Tirhuta */
    SBScriptWARA = 0x7D,    /**< Warang_Citi */

    /* Unicode 8.0 */
    SBScriptAHOM = 0x7E,    /**< Ahom */
    SBScriptHATR = 0x7F,    /**< Hatran */
    SBScriptHLUW = 0x80,    /**< Anatolian_Hieroglyphs */
    SBScriptHUNG = 0x81,    /**< Old_Hungarian */
    SBScriptMULT = 0x82,    /**< Multani */
    SBScriptSGNW = 0x83,    /**< SignWriting */

    /* Unicode 9.0 */
    SBScriptADLM = 0x84,    /**< Adlam */
    SBScriptBHKS = 0x85,    /**< Bhaiksuki */
    SBScriptMARC = 0x86,    /**< Marchen */
    SBScriptNEWA = 0x87,    /**< Newa */
    SBScriptOSGE = 0x88,    /**< Osage */
    SBScriptTANG = 0x89,    /**< Tangut */

    /* Unicode 10.0 */
    SBScriptGONM = 0x8A,    /**< Masaram_Gondi */
    SBScriptNSHU = 0x8B,    /**< Nushu */
    SBScriptSOYO = 0x8C,    /**< Soyombo */
    SBScriptZANB = 0x8D     /**< Zanabazar_Square */
};
/**
 * A type to represent the script of a character.
 */
typedef SBUInt8 SBScript;

#endif
