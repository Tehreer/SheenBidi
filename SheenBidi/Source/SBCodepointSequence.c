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

#include <SBConfig.h>

#include <stddef.h>
#include <stdlib.h>

#include "SBAssert.h"
#include "SBBase.h"
#include "SBCodepointSequence.h"

static SBCodepointSequenceRef SBCodepointSequenceCreateWithEncoding(SBStringEncoding stringEncoding,
    const void *stringBuffer, SBUInteger stringLength);

static SBCodepoint _SBGetUTF8CodepointAt(SBCodepointSequenceRef codepointSequence, SBUInteger *stringIndex);
static SBCodepoint _SBGetUTF8CodepointBefore(SBCodepointSequenceRef codepointSequence, SBUInteger *stringIndex);
static SBCodepoint _SBGetUTF16CodepointAt(SBCodepointSequenceRef codepointSequence, SBUInteger *stringIndex);
static SBCodepoint _SBGetUTF16CodepointBefore(SBCodepointSequenceRef codepointSequence, SBUInteger *stringIndex);
static SBCodepoint _SBGetUTF32CodepointAt(SBCodepointSequenceRef codepointSequence, SBUInteger *stringIndex);
static SBCodepoint _SBGetUTF32CodepointBefore(SBCodepointSequenceRef codepointSequence, SBUInteger *stringIndex);

SBCodepointSequenceRef SBCodepointSequenceCreateWithUTF8String(const SBUInt8 *string, SBUInteger length)
{
    return SBCodepointSequenceCreateWithEncoding(SBStringEncodingUTF8, string, length);
}

SBCodepointSequenceRef SBCodepointSequenceCreateWithUTF16String(const SBUInt16 *string, SBUInteger length)
{
    return SBCodepointSequenceCreateWithEncoding(SBStringEncodingUTF16, string, length);
}

SBCodepointSequenceRef SBCodepointSequenceCreateWithUTF32String(const SBUInt32 *string, SBUInteger length)
{
    return SBCodepointSequenceCreateWithEncoding(SBStringEncodingUTF32, string, length);
}

SBUInteger SBCodepointSequenceGetStringLength(SBCodepointSequenceRef codepointSequence)
{
    return codepointSequence->stringLength;
}

SBCodepoint SBCodepointSequenceGetCodepointBefore(SBCodepointSequenceRef codepointSequence, SBUInteger *stringIndex)
{
    SBCodepoint codepoint = SBCodepointInvalid;

    if ((*stringIndex - 1) < codepointSequence->stringLength) {
        switch (codepointSequence->stringEncoding) {
            case SBStringEncodingUTF8:
                codepoint = _SBGetUTF8CodepointBefore(codepointSequence, stringIndex);
                break;

            case SBStringEncodingUTF16:
                codepoint = _SBGetUTF16CodepointBefore(codepointSequence, stringIndex);
                break;

            case SBStringEncodingUTF32:
                codepoint = _SBGetUTF32CodepointBefore(codepointSequence, stringIndex);
                break;
        }
    }

    return codepoint;
}

SBCodepoint SBCodepointSequenceGetCodepointAt(SBCodepointSequenceRef codepointSequence, SBUInteger *stringIndex)
{
    SBCodepoint codepoint = SBCodepointInvalid;

    if (*stringIndex < codepointSequence->stringLength) {
        switch (codepointSequence->stringEncoding) {
            case SBStringEncodingUTF8:
                codepoint = _SBGetUTF8CodepointAt(codepointSequence, stringIndex);
                break;

            case SBStringEncodingUTF16:
                codepoint = _SBGetUTF16CodepointAt(codepointSequence, stringIndex);
                break;

            case SBStringEncodingUTF32:
                codepoint = _SBGetUTF32CodepointAt(codepointSequence, stringIndex);
                break;
        }
    }

    return codepoint;
}

SBCodepointSequenceRef SBCodepointSequenceRetain(SBCodepointSequenceRef codepointSequence)
{
    if (codepointSequence) {
        ++codepointSequence->_retainCount;
    }

    return codepointSequence;
}

void SBCodepointSequenceRelease(SBCodepointSequenceRef codepointSequence)
{
    if (codepointSequence && --codepointSequence->_retainCount == 0) {
        free(codepointSequence);
    }
}

static SBCodepointSequenceRef SBCodepointSequenceCreateWithEncoding(SBStringEncoding stringEncoding,
    const void *stringBuffer, SBUInteger stringLength)
{
    SBCodepointSequenceRef codepointSequence = NULL;

    if (stringBuffer && stringLength > 0) {
        codepointSequence = malloc(sizeof(SBCodepointSequence));
        codepointSequence->stringEncoding = stringEncoding;
        codepointSequence->stringBuffer = stringBuffer;
        codepointSequence->stringLength = stringLength;
        codepointSequence->_retainCount = 1;
    }

    return codepointSequence;
}

static SBCodepoint _SBGetUTF8CodepointAt(SBCodepointSequenceRef codepointSequence, SBUInteger *stringIndex)
{
    const SBUInt8 *utf8String = codepointSequence->stringBuffer;
    SBUInteger remaining = codepointSequence->stringLength - *stringIndex;
    SBCodepoint header;
    SBCodepoint result;

    /**
     * Reference: https://en.wikipedia.org/wiki/UTF-8
     *
     * +======+=========+==========+=======+==========+==========+==========+==========+
     * | Bits | First   | Last     | Bytes | Byte 1   | Byte 2   | Byte 3   | Byte 4   |
     * +======+=========+==========+=======+==========+==========+==========+==========+
     * |  7   | U+0000  | U+007F   |   1   | 0xxxxxxx |          |          |          |
     * +------+---------+----------+-------+----------+----------+----------+----------+
     * |  11  | U+0080  | U+07FF   |   2   | 110xxxxx | 10xxxxxx |          |          |
     * +------+---------+----------+-------+----------+----------+----------+----------+
     * |  16  | U+0800  | U+FFFF   |   3   | 1110xxxx | 10xxxxxx | 10xxxxxx |          |
     * +------+---------+----------+-------+----------+----------+----------+----------+
     * |  21  | U+10000 | U+1FFFFF |   4   | 11110xxx | 10xxxxxx | 10xxxxxx | 10xxxxxx |
     * +------+---------+----------+-------+----------+----------+----------+----------+
     */

    header = utf8String[*stringIndex];
    result = SBCodepointFaulty;

    *stringIndex += 1;

    if (header < 0x80) {
        result = header;
    } else if (SBUInt8InRange(header, 0xC2, 0xDF)) {
        if (remaining > 1) {
            SBCodepoint byte1 = header & 0xF;
            SBCodepoint byte2 = utf8String[*stringIndex + 0] - 0x80;

            if (byte2 <= 0x3F) {
                SBCodepoint codepoint = (byte1 << 6)
                                      | (byte2 << 0);

                result = codepoint;
                *stringIndex += 1;
            }
        }
    } else if (SBUInt8InRange(header, 0xE0, 0xEF)) {
        if (remaining > 2) {
            SBCodepoint byte1 = header & 0xF;
            SBCodepoint byte2 = utf8String[*stringIndex + 0] - 0x80;
            SBCodepoint byte3 = utf8String[*stringIndex + 1] - 0x80;

            if (byte2 <= 0x3F && byte3 <= 0x3F) {
                SBCodepoint codepoint = (byte1 << 12)
                                      | (byte2 <<  6)
                                      | (byte3 <<  0);

                if (codepoint > 0x0800 && !SBCodepointInRange(codepoint, 0xD800, 0xDFFF)) {
                    result = codepoint;
                    *stringIndex += 2;
                }
            }
        }
    } else if (SBUInt8InRange(header, 0xF0, 0xF4)) {
        if (remaining > 3) {
            SBCodepoint byte1 = header & 0x7;
            SBCodepoint byte2 = utf8String[*stringIndex + 0] - 0x80;
            SBCodepoint byte3 = utf8String[*stringIndex + 1] - 0x80;
            SBCodepoint byte4 = utf8String[*stringIndex + 2] - 0x80;

            if (byte2 <= 0x3F && byte3 <= 0x3F && byte4 <= 0x3F) {
                SBCodepoint codepoint = (byte1 << 18)
                                      | (byte2 << 12)
                                      | (byte3 <<  6)
                                      | (byte4 <<  0);

                if (SBCodepointInRange(codepoint, 0x10000, 0x10FFFF)) {
                    result = codepoint;
                    *stringIndex += 3;
                }
            }
        }
    }

    return result;
}

static SBCodepoint _SBGetUTF8CodepointBefore(SBCodepointSequenceRef codepointSequence, SBUInteger *stringIndex)
{
    const SBUInt8 *utf8String = codepointSequence->stringBuffer;
    SBUInteger startIndex = *stringIndex;
    SBUInteger nextIndex;
    SBUInteger looper;
    SBCodepoint result;

    looper = 4;

    while (--looper && startIndex--) {
        SBUInt8 codeunit = utf8String[startIndex];

        if ((codeunit & 0xC0) != 0x80) {
            break;
        }
    }

    nextIndex = startIndex;
    result = _SBGetUTF8CodepointAt(codepointSequence, &nextIndex);

    if (nextIndex == *stringIndex) {
        *stringIndex = startIndex;
    } else {
        *stringIndex -= 1;
        result = SBCodepointFaulty;
    }

    return result;
}

static SBCodepoint _SBGetUTF16CodepointAt(SBCodepointSequenceRef codepointSequence, SBUInteger *stringIndex)
{
    const SBUInt16 *utf16String = codepointSequence->stringBuffer;
    SBUInteger remaining = codepointSequence->stringLength - *stringIndex;
    SBCodepoint header;
    SBCodepoint result;

    header = utf16String[*stringIndex];
    result = SBCodepointFaulty;

    *stringIndex += 1;

    if (!SBCodepointIsSurrogate(header)) {
        result = header;
    } else if (header <= 0xDBFF) {
        if (remaining > 1) {
            SBCodepoint high = header;
            SBCodepoint low = utf16String[*stringIndex];

            if (SBCodepointInRange(low, 0xDC00, 0xDFFF)) {
                result = (high << 10) + low - ((0xD800 << 10) + 0xDC00 - 0x10000);
                *stringIndex += 1;
            }
        }
    }

    return result;
}

static SBCodepoint _SBGetUTF16CodepointBefore(SBCodepointSequenceRef codepointSequence, SBUInteger *stringIndex)
{
    const SBUInt16 *utf16String = codepointSequence->stringBuffer;
    SBUInteger remaining;
    SBCodepoint trailer;
    SBCodepoint result;

    *stringIndex -= 1;
    remaining = codepointSequence->stringLength - *stringIndex;

    trailer = utf16String[*stringIndex];
    result = SBCodepointFaulty;

    if (!SBCodepointIsSurrogate(trailer)) {
        result = trailer;
    } else if (trailer >= 0xDC00) {
        if (remaining > 1) {
            SBCodepoint low = trailer;
            SBCodepoint high = utf16String[*stringIndex - 1];

            if (SBCodepointInRange(high, 0xD800, 0xDBFF)) {
                result = (high << 10) + low - ((0xD800 << 10) + 0xDC00 - 0x10000);
                *stringIndex -= 1;
            }
        }
    }
    
    return result;
}

static SBCodepoint _SBGetUTF32CodepointAt(SBCodepointSequenceRef codepointSequence, SBUInteger *stringIndex)
{
    const SBUInt32 *utf32String = codepointSequence->stringBuffer;
    SBCodepoint header;
    SBCodepoint result;

    header = utf32String[*stringIndex];
    result = SBCodepointFaulty;

    *stringIndex += 1;

    if (SBCodepointIsValid(header)) {
        result = header;
    }
    
    return result;
}

static SBCodepoint _SBGetUTF32CodepointBefore(SBCodepointSequenceRef codepointSequence, SBUInteger *stringIndex)
{
    const SBUInt32 *utf32String = codepointSequence->stringBuffer;
    SBCodepoint header;
    SBCodepoint result;

    *stringIndex -= 1;

    header = utf32String[*stringIndex];
    result = SBCodepointFaulty;

    if (SBCodepointIsValid(header)) {
        result = header;
    }

    return result;
}
