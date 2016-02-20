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

#include <SBTypes.h>

enum {
    SBCharTypeNil = 0x00,

    SBCharTypeL   = 0x01,   /**< Strong: Left-to-Right */
    SBCharTypeR   = 0x02,   /**< Strong: Right-to-Left */
    SBCharTypeAL  = 0x03,   /**< Strong: Right-to-Left Arabic */

    SBCharTypeEN  = 0x04,   /**< Weak: European Number */
    SBCharTypeAN  = 0x05,   /**< Weak: Arabic Number */
    SBCharTypeET  = 0x06,   /**< Weak: European Number Terminator */
    SBCharTypeNSM = 0x07,   /**< Weak: Non-Spacing Mark */
    SBCharTypeBN  = 0x08,   /**< Weak: Boundary Neutral */
    SBCharTypeCS  = 0x09,   /**< Weak: Common Number Separator */
    SBCharTypeES  = 0x0A,   /**< Weak: European Number Separator */

    SBCharTypeB   = 0x0B,   /**< Neutral: Paragraph Separator */
    SBCharTypeS   = 0x0C,   /**< Neutral: Segment Separator */
    SBCharTypeWS  = 0x0D,   /**< Neutral: White Space */
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

#define _SBCharTypeInRange(t, s, e)         \
(                                           \
    (SBCharType)((t) - (s))                 \
 <= (SBCharType)((e) - (s))                 \
)

#define SBCharTypeIsStrong(t)               _SBCharTypeInRange(t, SBCharTypeL, SBCharTypeAL)
#define SBCharTypeIsWeak(t)                 _SBCharTypeInRange(t, SBCharTypeEN, SBCharTypeES)
#define SBCharTypeIsNeutral(t)              _SBCharTypeInRange(t, SBCharTypeB, SBCharTypeON)
#define SBCharTypeIsFormat(t)               _SBCharTypeInRange(t, SBCharTypeLRI, SBCharTypePDF)

#define SBCharTypeIsNumber(t)               _SBCharTypeInRange(t, SBCharTypeEN, SBCharTypeAN)
#define SBCharTypeIsSeparator(t)            _SBCharTypeInRange(t, SBCharTypeCS, SBCharTypeS)
#define SBCharTypeIsIsolate(t)              _SBCharTypeInRange(t, SBCharTypeLRI, SBCharTypePDI)

#define SBCharTypeIsStrongOrNumber(t)       _SBCharTypeInRange(t, SBCharTypeL, SBCharTypeAN)
#define SBCharTypeIsNumberSeparator(t)      _SBCharTypeInRange(t, SBCharTypeCS, SBCharTypeES)
#define SBCharTypeIsIsolateInitiator(t)     _SBCharTypeInRange(t, SBCharTypeLRI, SBCharTypeFSI)
#define SBCharTypeIsIsolateTerminator(t)    _SBCharTypeIsEqual(t, SBCharTypePDI)
#define SBCharTypeIsNeutralOrIsolate(t)     _SBCharTypeInRange(t, SBCharTypeB, SBCharTypePDI)

#endif
