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

static SBCodepointSequenceRef SBCodepointSequenceCreateWithEncoding(SBEncoding encoding,
    void *buffer, SBUInteger length);

static SBCodepoint _SBGetUTF32CodepointAt(SBCodepointSequenceRef codepointSequence, SBUInteger *index);
static SBCodepoint _SBGetUTF16CodepointAt(SBCodepointSequenceRef codepointSequence, SBUInteger *index);
static SBCodepoint _SBGetUTF8CodepointAt(SBCodepointSequenceRef codepointSequence, SBUInteger *index);

SBCodepointSequenceRef SBCodepointSequenceCreateWithUTF8Buffer(SBUInt8 *buffer, SBUInteger length)
{
    return SBCodepointSequenceCreateWithEncoding(SBEncodingUTF8, buffer, length);
}

SBCodepointSequenceRef SBCodepointSequenceCreateWithUTF16Buffer(SBUInt16 *buffer, SBUInteger length)
{
    return SBCodepointSequenceCreateWithEncoding(SBEncodingUTF16, buffer, length);
}

SBCodepointSequenceRef SBCodepointSequenceCreateWithUTF32Buffer(SBUInt32 *buffer, SBUInteger length)
{
    return SBCodepointSequenceCreateWithEncoding(SBEncodingUTF32, buffer, length);
}

SBCodepoint SBCodepointSequenceGetCodepointAt(SBCodepointSequenceRef codepointSequence, SBUInteger *bufferIndex)
{
    SBCodepoint codepoint = SBCodepointInvalid;

    if (bufferIndex && *bufferIndex < codepointSequence->length) {
        switch (codepointSequence->_encoding) {
            case SBEncodingUTF8:
                codepoint = _SBGetUTF8CodepointAt(codepointSequence, bufferIndex);
                break;

            case SBEncodingUTF16:
                codepoint = _SBGetUTF16CodepointAt(codepointSequence, bufferIndex);
                break;

            case SBEncodingUTF32:
                codepoint = _SBGetUTF32CodepointAt(codepointSequence, bufferIndex);
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

static SBCodepointSequenceRef SBCodepointSequenceCreateWithEncoding(SBEncoding encoding,
    void *buffer, SBUInteger length)
{
    SBCodepointSequenceRef codepointSequence = NULL;

    if (buffer && length > 0) {
        codepointSequence = malloc(sizeof(SBCodepointSequence));
        codepointSequence->_encoding = encoding;
        codepointSequence->buffer = buffer;
        codepointSequence->length = length;
        codepointSequence->_retainCount = 1;
    }

    return codepointSequence;
}

static SBCodepoint _SBGetUTF8CodepointAt(SBCodepointSequenceRef codepointSequence, SBUInteger *index)
{
    SBUInt8 *buffer = codepointSequence->buffer;
    SBUInteger remaining = codepointSequence->length - *index;
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

    header = buffer[*index];
    result = SBCodepointFaulty;

    *index += 1;

    if (header < 0x80) {
        result = header;
    } else if (SBUInt8InRange(header, 0xC2, 0xDF)) {
        if (remaining > 1) {
            SBCodepoint byte1 = header & 0xF;
            SBCodepoint byte2 = buffer[*index + 0] - 0x80;

            if (byte2 <= 0x3F) {
                SBCodepoint codepoint = (byte1 << 6)
                                      | (byte2 << 0);

                result = codepoint;
                *index += 1;
            }
        }
    } else if (SBUInt8InRange(header, 0xE0, 0xEF)) {
        if (remaining > 2) {
            SBCodepoint byte1 = header & 0xF;
            SBCodepoint byte2 = buffer[*index + 0] - 0x80;
            SBCodepoint byte3 = buffer[*index + 1] - 0x80;

            if (byte2 <= 0x3F && byte3 <= 0x3F) {
                SBCodepoint codepoint = (byte1 << 12)
                                      | (byte2 <<  6)
                                      | (byte3 <<  0);

                if (codepoint > 0x0800 && !SBCodepointInRange(codepoint, 0xD800, 0xDFFF)) {
                    result = codepoint;
                    *index += 2;
                }
            }
        }
    } else if (SBUInt8InRange(header, 0xF0, 0xF4)) {
        if (remaining > 3) {
            SBCodepoint byte1 = header & 0x7;
            SBCodepoint byte2 = buffer[*index + 0] - 0x80;
            SBCodepoint byte3 = buffer[*index + 1] - 0x80;
            SBCodepoint byte4 = buffer[*index + 2] - 0x80;

            if (byte2 <= 0x3F && byte3 <= 0x3F && byte4 <= 0x3F) {
                SBCodepoint codepoint = (byte1 << 18)
                                      | (byte2 << 12)
                                      | (byte3 <<  6)
                                      | (byte4 <<  0);

                if (SBCodepointInRange(codepoint, 0x10000, 0x10FFFF)) {
                    result = codepoint;
                    *index += 3;
                }
            }
        }
    }

    return result;
}

static SBCodepoint _SBGetUTF16CodepointAt(SBCodepointSequenceRef codepointSequence, SBUInteger *index)
{
    SBUInt16 *buffer = codepointSequence->buffer;
    SBUInteger remaining = codepointSequence->length - *index;
    SBCodepoint header;
    SBCodepoint result;

    header = buffer[*index];
    result = SBCodepointFaulty;

    *index += 1;

    if (!SBUInt16InRange(header, 0xD800, 0xDFFF)) {
        result = header;
    } else if (header <= 0xDBFF) {
        if (remaining > 1) {
            SBCodepoint high = header;
            SBCodepoint low = buffer[*index];

            if (SBCodepointInRange(low, 0xDC00, 0xDFFF)) {
                result = (high << 10) + low - ((0xD800 << 10) + 0xDC00 - 0x10000);
                *index += 1;
            }
        }
    }

    return result;
}

static SBCodepoint _SBGetUTF32CodepointAt(SBCodepointSequenceRef codepointSequence, SBUInteger *index)
{
    SBUInt32 *buffer = codepointSequence->buffer;
    SBCodepoint header;
    SBCodepoint result;

    header = buffer[*index];
    result = SBCodepointFaulty;

    *index += 1;

    if (!SBCodepointInRange(header, 0xD800, 0xDFFF) && header <= 0x10FFFF) {
        result = header;
    }
    
    return result;
}
