/*
 * Copyright (C) 2014-2018 Muhammad Tayyab Akram
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

#include "SBBidiType.h"
#include "SBGeneralCategory.h"
#include "SBScript.h"

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
 * A type to represent the bidirectional type of a character.
 */
typedef SBUInt8 SBBidiType;

/**
 * Checks whether specified bidirectional type is strong.
 */
#define SBBidiTypeIsStrong(t)               SBUInt8InRange(t, SBBidiTypeL, SBBidiTypeAL)

/**
 * Checks whether specified bidirectional type is weak.
 */
#define SBBidiTypeIsWeak(t)                 SBUInt8InRange(t, SBBidiTypeBN, SBBidiTypeCS)

/**
 * Checks whether specified bidirectional type is neutral.
 */
#define SBBidiTypeIsNeutral(t)              SBUInt8InRange(t, SBBidiTypeWS, SBBidiTypeON)

/**
 * Checks whether specified bidirectional type is format.
 */
#define SBBidiTypeIsFormat(t)               SBUInt8InRange(t, SBBidiTypeLRI, SBBidiTypePDF)


/**
 * A type to represent the general category of a character.
 */
typedef SBUInt8 SBGeneralCategory;

/**
 * Checks whether specified general category is letter.
 */
#define SBGeneralCategoryIsLetter(gc)       SBUInt8InRange(gc, SBGeneralCategoryLU, SBGeneralCategoryLO)

/**
 * Checks whether specified general category is mark.
 */
#define SBGeneralCategoryIsMark(gc)         SBUInt8InRange(gc, SBGeneralCategoryMN, SBGeneralCategoryME)

/**
 * Checks whether specified general category is number.
 */
#define SBGeneralCategoryIsNumber(gc)       SBUInt8InRange(gc, SBGeneralCategoryND, SBGeneralCategoryNO)

/**
 * Checks whether specified general category is punctuation.
 */
#define SBGeneralCategoryIsPunctuation(gc)  SBUInt8InRange(gc, SBGeneralCategoryPC, SBGeneralCategoryPO)

/**
 * Checks whether specified general category is symbol.
 */
#define SBGeneralCategoryIsSymbol(gc)       SBUInt8InRange(gc, SBGeneralCategorySM, SBGeneralCategorySO)

/**
 * Checks whether specified general category is separator.
 */
#define SBGeneralCategoryIsSeparator(gc)    SBUInt8InRange(gc, SBGeneralCategoryZS, SBGeneralCategoryZP)

/**
 * Checks whether specified general category is other.
 */
#define SBGeneralCategoryIsOther(gc)        SBUInt8InRange(gc, SBGeneralCategoryCC, SBGeneralCategoryCN)


/**
 * A type to represent the script of a character.
 */
typedef SBUInt8 SBScript;


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
 *      The bidirectional type of specified code point.
 */
SBBidiType SBCodepointGetBidiType(SBCodepoint codepoint);

/**
 * Returns the mirror of a code point.
 *
 * @param codepoint
 *      The code point whose mirror is returned.
 * @return
 *      The mirror of specified code point if available, 0 otherwise.
 */
SBCodepoint SBCodepointGetMirror(SBCodepoint codepoint);

/**
 * Returns the script of a code point.
 *
 * @param codepoint
 *      The code point whose script is returned.
 * @return
 *      The script of specified code point.
 */
SBScript SBCodepointGetScript(SBCodepoint codepoint);

#endif
