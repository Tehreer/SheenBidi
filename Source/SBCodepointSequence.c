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

static SBCodepoint _SBGetUTF8CodepointAt(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex);
static SBCodepoint _SBGetUTF8CodepointBefore(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex);
static SBCodepoint _SBGetUTF16CodepointAt(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex);
static SBCodepoint _SBGetUTF16CodepointBefore(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex);
static SBCodepoint _SBGetUTF32CodepointAt(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex);
static SBCodepoint _SBGetUTF32CodepointBefore(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex);

SB_INTERNAL SBBoolean SBCodepointSequenceIsValid(const SBCodepointSequence *codepointSequence)
{
    if (codepointSequence) {
        SBBoolean encodingValid = SBFalse;

        switch (codepointSequence->stringEncoding) {
        case SBStringEncodingUTF8:
        case SBStringEncodingUTF16:
        case SBStringEncodingUTF32:
            encodingValid = SBTrue;
            break;
        }

        return (encodingValid && codepointSequence->stringBuffer && codepointSequence->stringLength > 0);
    }

    return SBFalse;
}

SBCodepoint SBCodepointSequenceGetCodepointBefore(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex)
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

SBCodepoint SBCodepointSequenceGetCodepointAt(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex)
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

static SBCodepoint _SBGetUTF8CodepointAt(const SBCodepointSequence *sequence, SBUInteger *index)
{
    const SBUInt8 *buffer = sequence->stringBuffer;
    SBUInteger length = sequence->stringLength;
    SBCodepoint result;
    SBUInt8 lead;

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

    lead = buffer[(*index)++];
    result = SBCodepointFaulty;

    if (lead < 0x80) {
        result = lead;
    } else if (lead < 0xC0) {
        /* Lone continuation byte. */
    } else if (lead < 0xE0) {
        SBCodepoint codepoint;
        SBCodepoint bytes[2];

        bytes[0] = lead & 0x1F;

        if (((*index) < length && (bytes[1] = buffer[*index] - 0x80) < 0x40)) {
           ++(*index);

            codepoint = (bytes[0] << 6)
                      | (bytes[1] << 0);

            if (codepoint >= 0x0080) {
                result = codepoint;
            }
        }
    } else if (lead < 0xF0) {
        SBCodepoint codepoint;
        SBCodepoint bytes[3];

        bytes[0] = lead & 0xF;

        if (((*index) < length && (bytes[1] = buffer[*index] - 0x80) < 0x40) &&
          (++(*index) < length && (bytes[2] = buffer[*index] - 0x80) < 0x40)) {
           ++(*index);

            codepoint = (bytes[0] << 12)
                      | (bytes[1] <<  6)
                      | (bytes[2] <<  0);

            if (codepoint >= 0x0800 && !SBCodepointInRange(codepoint, 0xD800, 0xDFFF)) {
                result = codepoint;
            }
        }
    } else if (lead < 0xF8) {
        SBCodepoint codepoint;
        SBCodepoint bytes[4];

        bytes[0] = lead & 0x7;

        if (((*index) < length && (bytes[1] = buffer[*index] - 0x80) < 0x40) &&
          (++(*index) < length && (bytes[2] = buffer[*index] - 0x80) < 0x40) &&
          (++(*index) < length && (bytes[3] = buffer[*index] - 0x80) < 0x40)) {
           ++(*index);

            codepoint = (bytes[0] << 18)
                      | (bytes[1] << 12)
                      | (bytes[2] <<  6)
                      | (bytes[3] <<  0);

            if (SBCodepointInRange(codepoint, 0x10000, 0x10FFFF)) {
                result = codepoint;
            }
        }
    } else if (lead < 0xFC) {
        /* Invalid byte sequence. */

        if (((*index) < length && SBUInt8InRange(buffer[*index], 0x80, 0xBF)) &&
          (++(*index) < length && SBUInt8InRange(buffer[*index], 0x80, 0xBF)) &&
          (++(*index) < length && SBUInt8InRange(buffer[*index], 0x80, 0xBF)) &&
          (++(*index) < length && SBUInt8InRange(buffer[*index], 0x80, 0xBF))) {
          ++(*index);
            /* The above statements will skip valid continuation bytes. */
        }
    } else if (lead < 0xFE) {
        /* Invalid byte sequence. */

        if (((*index) < length && SBUInt8InRange(buffer[*index], 0x80, 0xBF)) &&
          (++(*index) < length && SBUInt8InRange(buffer[*index], 0x80, 0xBF)) &&
          (++(*index) < length && SBUInt8InRange(buffer[*index], 0x80, 0xBF)) &&
          (++(*index) < length && SBUInt8InRange(buffer[*index], 0x80, 0xBF)) &&
          (++(*index) < length && SBUInt8InRange(buffer[*index], 0x80, 0xBF))) {
           ++(*index);
            /* The above statements will skip valid continuation bytes. */
        }
    }

    return result;
}

static SBCodepoint _SBGetUTF8CodepointBefore(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex)
{
    const SBUInt8 *buffer = codepointSequence->stringBuffer;
    SBUInteger startIndex = *stringIndex;
    SBUInteger nextIndex;
    SBUInteger continuation;
    SBCodepoint result;

    continuation = 7;

    while (--continuation && --startIndex) {
        SBUInt8 codeunit = buffer[startIndex];

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

static SBCodepoint _SBGetUTF16CodepointAt(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex)
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

static SBCodepoint _SBGetUTF16CodepointBefore(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex)
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

static SBCodepoint _SBGetUTF32CodepointAt(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex)
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

static SBCodepoint _SBGetUTF32CodepointBefore(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex)
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
