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

#ifndef _SB_INTERNAL_CHAR_TYPE_H
#define _SB_INTERNAL_CHAR_TYPE_H

#include "SBTypes.h"

enum {
    SBCharTypeNil = 0x00,

    SBCharTypeL   = 0x01,   /**< Strong: Left-to-Right */
    SBCharTypeR   = 0x02,   /**< Strong: Right-to-Left */
    SBCharTypeAL  = 0x03,   /**< Strong: Right-to-Left Arabic */

    SBCharTypeBN  = 0x04,   /**< Weak: Boundary Neutral */
    SBCharTypeNSM = 0x05,   /**< Weak: Non-Spacing Mark */
    SBCharTypeAN  = 0x06,   /**< Weak: Arabic Number */
    SBCharTypeEN  = 0x07,   /**< Weak: European Number */
    SBCharTypeET  = 0x08,   /**< Weak: European Number Terminator */
    SBCharTypeES  = 0x09,   /**< Weak: European Number Separator */
    SBCharTypeCS  = 0x0A,   /**< Weak: Common Number Separator */

    SBCharTypeWS  = 0x0B,   /**< Neutral: White Space */
    SBCharTypeS   = 0x0C,   /**< Neutral: Segment Separator */
    SBCharTypeB   = 0x0D,   /**< Neutral: Paragraph Separator */
    SBCharTypeON  = 0x0E,   /**< Neutral: Other Neutral */

    SBCharTypeLRI = 0x0F,   /**< Format: Left-to-Right Isolate */
    SBCharTypeRLI = 0x10,   /**< Format: Right-to-Left Isolate */
    SBCharTypeFSI = 0x11,   /**< Format: First Strong Isolate */
    SBCharTypePDI = 0x12,   /**< Format: Pop Directional Isolate */
    SBCharTypeLRE = 0x13,   /**< Format: Left-to-Right Embedding */
    SBCharTypeRLE = 0x14,   /**< Format: Right-to-Left Embedding */
    SBCharTypeLRO = 0x15,   /**< Format: Left-to-Right Override */
    SBCharTypeRLO = 0x16,   /**< Format: Right-to-Left Override */
    SBCharTypePDF = 0x17    /**< Format: Pop Directional Formatting */

};
typedef SBUInt8 SBCharType;

#define _SBCharTypeIsEqual(t1, t2)          \
(                                           \
 (t1) == (t2)                               \
)

#define _SBCharTypeInRange(t, s, e)         SBUInt8InRange(t, s, e)

#define SBCharTypeIsStrong(t)               _SBCharTypeInRange(t, SBCharTypeL, SBCharTypeAL)
#define SBCharTypeIsWeak(t)                 _SBCharTypeInRange(t, SBCharTypeBN, SBCharTypeCS)
#define SBCharTypeIsNeutral(t)              _SBCharTypeInRange(t, SBCharTypeWS, SBCharTypeON)
#define SBCharTypeIsFormat(t)               _SBCharTypeInRange(t, SBCharTypeLRI, SBCharTypePDF)

#define SBCharTypeIsNumber(t)               _SBCharTypeInRange(t, SBCharTypeAN, SBCharTypeEN)
#define SBCharTypeIsIsolate(t)              _SBCharTypeInRange(t, SBCharTypeLRI, SBCharTypePDI)

#define SBCharTypeIsStrongOrNumber(t)       (SBCharTypeIsStrong(t) || SBCharTypeIsNumber(t))
#define SBCharTypeIsNumberSeparator(t)      _SBCharTypeInRange(t, SBCharTypeES, SBCharTypeCS)
#define SBCharTypeIsIsolateInitiator(t)     _SBCharTypeInRange(t, SBCharTypeLRI, SBCharTypeFSI)
#define SBCharTypeIsIsolateTerminator(t)    _SBCharTypeIsEqual(t, SBCharTypePDI)
#define SBCharTypeIsNeutralOrIsolate(t)     _SBCharTypeInRange(t, SBCharTypeWS, SBCharTypePDI)

#endif
