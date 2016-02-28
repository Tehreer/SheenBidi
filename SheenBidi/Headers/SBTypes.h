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

#ifndef _SB_PUBLIC_TYPES_H
#define _SB_PUBLIC_TYPES_H

#include <stddef.h>
#include <stdint.h>

/**
 * A type to represent an 8-bit signed integer.
 */
typedef int8_t          SBInt8;

/**
 * A type to represent a 16-bit signed integer.
 */
typedef int16_t         SBInt16;

/**
 * A type to represent a 32-bit signed integer.
 */
typedef int32_t         SBInt32;

/**
 * A type to represent an 8-bit unsigned integer.
 */
typedef uint8_t         SBUInt8;

/**
 * A type to represent a 16-bit unsigned integer.
 */
typedef uint16_t        SBUInt16;

/**
 * A type to represent a 32-bit unsigned integer.
 */
typedef uint32_t        SBUInt32;

/**
 * A signed integer type whose width is equal to the width of the machine word.
 */
typedef intptr_t        SBInteger;

/**
 * An unsigned integer type whose width is equal to the width of the machine word.
 */
typedef uintptr_t       SBUInteger;

/**
 * A type to represent a boolean value.
 */
enum {
    SBFalse = 0, /**< A value representing the false state. */
    SBTrue  = 1  /**< A value representing the true state. */
};
typedef SBUInt8         SBBoolean;

/**
 * A type to represent a unicode code point.
 */
typedef SBUInt32        SBCodepoint;

/**
 * A type to represent a bidi level.
 */
typedef SBUInt8         SBLevel;

/**
 * Finds the mirror of the provided code point.
 * @param codepoint
 *      The code point whose mirror you want to find.
 * @return
 *      The mirror of the provided code point if available, 0 otherwise.
 */
SBCodepoint SBCodepointGetMirror(SBCodepoint codepoint);

#endif
