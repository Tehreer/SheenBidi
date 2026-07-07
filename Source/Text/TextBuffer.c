/*
 * Copyright (C) 2026 Muhammad Tayyab Akram
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

#include <API/SBBase.h>

#if SB_TEXT_API_SUPPORTED

#include <stddef.h>
#include <string.h>

#include <SheenBidi/SBCodepointSequence.h>

#include <Core/List.h>

#include "TextBuffer.h"

/**
 * Returns the size in bytes of a single code unit for the given encoding.
 */
static SBUInteger GetCodeUnitSize(SBStringEncoding encoding)
{
    switch (encoding) {
    case SBStringEncodingUTF8:
        return sizeof(SBUInt8);

    case SBStringEncodingUTF16:
        return sizeof(SBUInt16);

    case SBStringEncodingUTF32:
        return sizeof(SBUInt32);

    default:
        return 0;
    }
}

SB_INTERNAL void TextBufferInitialize(TextBufferRef buffer, SBStringEncoding encoding)
{
    buffer->encoding = encoding;
    ListInitialize(&buffer->codeUnits, GetCodeUnitSize(encoding));
}

SB_INTERNAL void TextBufferInitializeCopy(TextBufferRef buffer, TextBufferRef source)
{
    SBUInteger byteCount;

    TextBufferInitialize(buffer, source->encoding);

    ListReserveRange(&buffer->codeUnits, 0, source->codeUnits.count);

    byteCount = source->codeUnits.count * source->codeUnits.itemSize;
    memcpy(buffer->codeUnits.data, source->codeUnits.data, byteCount);
}

SB_INTERNAL void TextBufferFinalize(TextBufferRef buffer)
{
    ListFinalize(&buffer->codeUnits);
}

SB_INTERNAL SBUInteger TextBufferGetLength(TextBufferRef buffer)
{
    return buffer->codeUnits.count;
}

SB_INTERNAL SBUInteger TextBufferGetMaxCodeUnitsPerCodepoint(TextBufferRef buffer)
{
    switch (buffer->encoding) {
    case SBStringEncodingUTF8:
        return 4;

    case SBStringEncodingUTF16:
        return 2;

    case SBStringEncodingUTF32:
        return 1;

    default:
        return 0;
    }
}

SB_INTERNAL const void *TextBufferGetCodeUnitsPtr(TextBufferRef buffer, SBUInteger index)
{
    return ListGetPtr(&buffer->codeUnits, index);
}

SB_INTERNAL void TextBufferGetCodepointSequence(TextBufferRef buffer, SBCodepointSequence *sequence)
{
    sequence->stringEncoding = buffer->encoding;
    sequence->stringBuffer = buffer->codeUnits.data;
    sequence->stringLength = buffer->codeUnits.count;
}

SB_INTERNAL void TextBufferGetCodeUnits(TextBufferRef buffer, SBUInteger index, SBUInteger length,
    void *destination)
{
    SBUInteger byteCount = length * buffer->codeUnits.itemSize;
    const void *source = ListGetPtr(&buffer->codeUnits, index);

    memcpy(destination, source, byteCount);
}

SB_INTERNAL void TextBufferReplaceCodeUnits(TextBufferRef buffer, SBUInteger index, SBUInteger oldLength,
    const void *codeUnitBuffer, SBUInteger newLength)
{
    if (newLength > oldLength) {
        ListReserveRange(&buffer->codeUnits, index, newLength - oldLength);
    } else if (oldLength > newLength) {
        ListRemoveRange(&buffer->codeUnits, index, oldLength - newLength);
    }

    if (newLength > 0) {
        SBUInteger byteCount = newLength * buffer->codeUnits.itemSize;
        void *destination = ListGetPtr(&buffer->codeUnits, index);

        memcpy(destination, codeUnitBuffer, byteCount);
    }
}

SB_INTERNAL void TextBufferInsertCodeUnits(TextBufferRef buffer, SBUInteger index,
    const void *codeUnitBuffer, SBUInteger codeUnitCount)
{
    TextBufferReplaceCodeUnits(buffer, index, 0, codeUnitBuffer, codeUnitCount);
}

SB_INTERNAL void TextBufferDeleteCodeUnits(TextBufferRef buffer, SBUInteger index, SBUInteger length)
{
    TextBufferReplaceCodeUnits(buffer, index, length, NULL, 0);
}

#endif
