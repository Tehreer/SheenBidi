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

#ifndef _SB_PUBLIC_BASE_H
#define _SB_PUBLIC_BASE_H

#include <stddef.h>
#include <stdint.h>

/**
 * A type to represent an 8-bit signed integer.
 */
typedef int8_t                      SBInt8;

/**
 * A type to represent a 16-bit signed integer.
 */
typedef int16_t                     SBInt16;

/**
 * A type to represent a 32-bit signed integer.
 */
typedef int32_t                     SBInt32;

/**
 * A type to represent an 8-bit unsigned integer.
 */
typedef uint8_t                     SBUInt8;

/**
 * A type to represent a 16-bit unsigned integer.
 */
typedef uint16_t                    SBUInt16;

/**
 * A type to represent a 32-bit unsigned integer.
 */
typedef uint32_t                    SBUInt32;

/**
 * A signed integer type whose width is equal to the width of the machine word.
 */
typedef intptr_t                    SBInteger;

/**
 * An unsigned integer type whose width is equal to the width of the machine word.
 */
typedef uintptr_t                   SBUInteger;

/**
 * Constants that specify the states of a boolean.
 */
enum {
    SBFalse = 0, /**< A value representing the false state. */
    SBTrue  = 1  /**< A value representing the true state. */
};
/**
 * A type to represent a boolean value.
 */
typedef SBUInt8                     SBBoolean;

#define SBUInt8InRange(v, s, e)     \
(                                   \
    (SBUInt8)((v) - (s))            \
 <= (SBUInt8)((e) - (s))            \
)

#define SBUInt16InRange(v, s, e)    \
(                                   \
    (SBUInt16)((v) - (s))           \
 <= (SBUInt16)((e) - (s))           \
)

#define SBUInt32InRange(v, s, e)    \
(                                   \
    (SBUInt32)((v) - (s))           \
 <= (SBUInt32)((e) - (s))           \
)


/**
 * A type to represent a bidi level.
 */
typedef SBUInt8                     SBLevel;

/**
 * A value representing an invalid bidi level.
 */
#define SBLevelInvalid              0xFF

/**
 * A value representing maximum explicit embedding level.
 */
#define SBLevelMax                  125

/**
 * A value specifying to set base level to zero (left-to-right) if there is no strong character.
 */
#define SBLevelDefaultLTR           0xFE

/**
 * A value specifying to set base level to one (right-to-left) if there is no strong character.
 */
#define SBLevelDefaultRTL           0xFD


/**
 * Constants that specify the bidirectional types of a character.
 */
enum {
    SBBidiTypeNil = 0x00,

    SBBidiTypeL   = 0x01,   /**< Strong: Left-to-Right */
    SBBidiTypeR   = 0x02,   /**< Strong: Right-to-Left */
    SBBidiTypeAL  = 0x03,   /**< Strong: Right-to-Left Arabic */

    SBBidiTypeBN  = 0x04,   /**< Weak: Boundary Neutral */
    SBBidiTypeNSM = 0x05,   /**< Weak: Non-Spacing Mark */
    SBBidiTypeAN  = 0x06,   /**< Weak: Arabic Number */
    SBBidiTypeEN  = 0x07,   /**< Weak: European Number */
    SBBidiTypeET  = 0x08,   /**< Weak: European Number Terminator */
    SBBidiTypeES  = 0x09,   /**< Weak: European Number Separator */
    SBBidiTypeCS  = 0x0A,   /**< Weak: Common Number Separator */

    SBBidiTypeWS  = 0x0B,   /**< Neutral: White Space */
    SBBidiTypeS   = 0x0C,   /**< Neutral: Segment Separator */
    SBBidiTypeB   = 0x0D,   /**< Neutral: Paragraph Separator */
    SBBidiTypeON  = 0x0E,   /**< Neutral: Other Neutral */

    SBBidiTypeLRI = 0x0F,   /**< Format: Left-to-Right Isolate */
    SBBidiTypeRLI = 0x10,   /**< Format: Right-to-Left Isolate */
    SBBidiTypeFSI = 0x11,   /**< Format: First Strong Isolate */
    SBBidiTypePDI = 0x12,   /**< Format: Pop Directional Isolate */
    SBBidiTypeLRE = 0x13,   /**< Format: Left-to-Right Embedding */
    SBBidiTypeRLE = 0x14,   /**< Format: Right-to-Left Embedding */
    SBBidiTypeLRO = 0x15,   /**< Format: Left-to-Right Override */
    SBBidiTypeRLO = 0x16,   /**< Format: Right-to-Left Override */
    SBBidiTypePDF = 0x17    /**< Format: Pop Directional Formatting */
};
/**
 * A type to represent the bidirectional type of a character.
 */
typedef SBUInt8 SBBidiType;

#define SBBidiTypeIsStrong(t)               SBUInt8InRange(t, SBBidiTypeL, SBBidiTypeAL)
#define SBBidiTypeIsWeak(t)                 SBUInt8InRange(t, SBBidiTypeBN, SBBidiTypeCS)
#define SBBidiTypeIsNeutral(t)              SBUInt8InRange(t, SBBidiTypeWS, SBBidiTypeON)
#define SBBidiTypeIsFormat(t)               SBUInt8InRange(t, SBBidiTypeLRI, SBBidiTypePDF)


/**
 * A type to represent a unicode code point.
 */
typedef SBUInt32                    SBCodepoint;

/**
 * A value representing an invalid code point.
 */
#define SBCodepointInvalid          UINT32_MAX

/**
 * A value representing a faulty code point, used as a replacement by the decoder.
 */
#define SBCodepointFaulty           0xFFFD

/**
 * Returns the bidirectional type of a code point.
 *
 * @param codepoint
 *      The code point whose bidirectional type is returned.
 * @return
 *      The bidirectional type of the provided code point.
 */
SBBidiType SBCodepointGetBidiType(SBCodepoint codepoint);

/**
 * Returns the mirror of a code point.
 *
 * @param codepoint
 *      The code point whose mirror is returned.
 * @return
 *      The mirror of the provided code point if available, 0 otherwise.
 */
SBCodepoint SBCodepointGetMirror(SBCodepoint codepoint);

#endif
