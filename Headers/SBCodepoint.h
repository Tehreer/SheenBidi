/*
 * Copyright (C) 2018-2025 Muhammad Tayyab Akram
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

#ifndef _SB_PUBLIC_CODEPOINT_H
#define _SB_PUBLIC_CODEPOINT_H

#include "SBBase.h"
#include "SBBidiType.h"
#include "SBGeneralCategory.h"
#include "SBScript.h"

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
 * Maximum valid code point value.
 */
#define SBCodepointMax                      0x10FFFF

/**
 * Whether the code point value represents part of a UTF-16 surrogate pair.
 * Such code point values are invalid.
 */
#define SBCodepointIsSurrogate(c)           SBUInt32InRange(c, 0xD800, 0xDFFF)

/**
 * Whether this code point value is valid, i.e.:
 * 1. It is less than 0x10FFFF.
 * 2. It is not in the surrogate range (0xD800 to 0xDFFF).
 */
#define SBCodepointIsValid(c)               (!SBCodepointIsSurrogate(c) && (c) <= SBCodepointMax)

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
 * Returns the general category of a code point.
 *
 * @param codepoint
 *      The code point whose general category is returned.
 * @return
 *      The general category of specified code point.
 */
SBGeneralCategory SBCodepointGetGeneralCategory(SBCodepoint codepoint);

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


/**
 * Returns the code point at the given UTF-8 code unit index.
 *
 * @param buffer
 *      The buffer holding the code units.
 * @param length
 *      Length of the buffer.
 * @param stringIndex
 *      The index of code unit at which to get the code point. On output, it is set to point to the
 *      first code unit of next code point.
 * @return
 *      The code point at the given string index, or SBCodepointInvalid if stringIndex is larger
 *      than or equal to actual length of source string.
 */
SBCodepoint SBCodepointDecodeNextFromUTF8(const SBUInt8 *buffer, SBUInteger length, SBUInteger *stringIndex);

/**
 * Returns the code point before the given UTF-8 code unit index.
 *
 * @param buffer
 *      The buffer holding the code units.
 * @param length
 *      Length of the buffer.
 * @param stringIndex
 *      The index of code unit before which to get the code point. On output, it is set to point to
 *      the first code unit of returned code point.
 * @return
 *      The code point before the given string index, or SBCodepointInvalid if stringIndex is equal 
 *      to zero or larger than actual length of source string.
 */
SBCodepoint SBCodepointDecodePreviousFromUTF8(const SBUInt8 *buffer, SBUInteger length, SBUInteger *stringIndex);

/**
 * Returns the code point at the given UTF-16 code unit index.
 *
 * @param buffer
 *      The buffer holding the code units.
 * @param length
 *      Length of the buffer.
 * @param stringIndex
 *      The index of code unit at which to get the code point. On output, it is set to point to the
 *      first code unit of next code point.
 * @return
 *      The code point at the given string index, or SBCodepointInvalid if stringIndex is larger
 *      than or equal to actual length of source string.
 */
SBCodepoint SBCodepointDecodeNextFromUTF16(const SBUInt16 *buffer, SBUInteger length, SBUInteger *stringIndex);

/**
 * Returns the code point before the given UTF-16 code unit index.
 *
 * @param buffer
 *      The buffer holding the code units.
 * @param length
 *      Length of the buffer.
 * @param stringIndex
 *      The index of code unit before which to get the code point. On output, it is set to point to
 *      the first code unit of returned code point.
 * @return
 *      The code point before the given string index, or SBCodepointInvalid if stringIndex is equal 
 *      to zero or larger than actual length of source string.
 */
SBCodepoint SBCodepointDecodePreviousFromUTF16(const SBUInt16 *buffer, SBUInteger length, SBUInteger *stringIndex);

#endif
