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

typedef struct {
    SBUInt8 valid;
    SBUInt8 total;
    SBUInt8 start;
    SBUInt8 end;
} _SBUTF8State;

static const _SBUTF8State _SBUTF8StateTable[9] = {
    {1,0,0x00,0x00}, {0,0,0x00,0x00}, {1,2,0x80,0xBF}, {1,3,0xA0,0xBF}, {1,3,0x80,0xBF},
    {1,3,0x80,0x9F}, {1,4,0x90,0xBF}, {1,4,0x80,0xBF}, {1,4,0x80,0x8F}
};

static const SBUInt8 _SBUTF8LookupTable[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
/* LEAD: -- 80..BF -- */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1,
/* LEAD: -- C0..C1 -- */
    1, 1,
/* LEAD: -- C2..DF -- */
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
/* LEAD: -- E0..E0 -- */
    3,
/* LEAD: -- E1..EC -- */
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
/* LEAD: -- ED..ED -- */
    5,
/* LEAD: -- EE..EF -- */
    4, 4,
/* LEAD: -- F0..F0 -- */
    6,
/* LEAD: -- F1..F3 -- */
    7, 7, 7,
/* LEAD: -- F4..F4 -- */
    8,
/* LEAD: -- F5..F7 -- */
    1, 1, 1,
/* LEAD: -- F8..FB -- */
    1, 1, 1, 1,
/* LEAD: -- FC..FD -- */
    1, 1,
/* LEAD: -- FE..FF -- */
    1, 1
};

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
    SBUInt8 lead;
    _SBUTF8State state;
    SBUInteger limit;
    SBCodepoint codepoint;

    lead = buffer[*index];
    state = _SBUTF8StateTable[_SBUTF8LookupTable[lead]];
    limit = *index + state.total;

    if (limit > length) {
        limit = length;
        state.valid = SBFalse;
    }

    codepoint = lead & (0x7F >> state.total);

    while (++(*index) < limit) {
        SBUInt8 byte = buffer[*index];

        if (byte >= state.start && byte <= state.end) {
            codepoint = (codepoint << 6) | (byte & 0x3F);
        } else {
            state.valid = SBFalse;
            break;
        }

        state.start = 0x80;
        state.end = 0xBF;
    }

    if (state.valid) {
        return codepoint;
    }

    return SBCodepointFaulty;
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
