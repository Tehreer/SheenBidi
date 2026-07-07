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

#include <API/SBCodepoint.h>
#include <API/SBCodepointSequence.h>
#include <Core/List.h>
#include <Text/TextBuffer.h>

#include "BidiTypesBuffer.h"

static void DetermineChunkBidiTypes(BidiTypesBufferRef bidiTypesBuffer, TextBufferRef buffer,
    SBUInteger index, SBUInteger length)
{
    SBUInteger codeUnitCount = TextBufferGetLength(buffer);

    if (codeUnitCount > 0) {
        SBUInteger startIndex = index;
        SBUInteger endIndex = startIndex + length;
        SBStringEncoding encoding = buffer->encoding;
        const void *codeUnits = buffer->codeUnits.data;
        SBUInteger surround;
        SBCodepointSequence sequence;

        surround = TextBufferGetMaxCodeUnitsPerCodepoint(buffer);

        startIndex = (startIndex >= surround ? startIndex - surround : 0);
        endIndex = ((endIndex + surround) <= codeUnitCount ? endIndex + surround : codeUnitCount);
        endIndex -= 1;

        /* Align to code point boundaries */
        SBCodepointSkipToStart(codeUnits, codeUnitCount, encoding, &startIndex);
        SBCodepointSkipToEnd(codeUnits, codeUnitCount, encoding, &endIndex);

        sequence.stringEncoding = encoding;
        sequence.stringBuffer = SBCodepointGetBufferOffset(codeUnits, encoding, startIndex);
        sequence.stringLength = endIndex - startIndex;

        SBCodepointSequenceDetermineBidiTypes(&sequence, &bidiTypesBuffer->bidiTypes.items[startIndex]);
    }
}

SB_INTERNAL void BidiTypesBufferInitialize(BidiTypesBufferRef bidiTypesBuffer)
{
    ListInitialize(&bidiTypesBuffer->bidiTypes, sizeof(SBBidiType));
}

SB_INTERNAL void BidiTypesBufferCopyBidiTypes(BidiTypesBufferRef bidiTypesBuffer,
    const BidiTypesBuffer *source)
{
    ListRemoveAll(&bidiTypesBuffer->bidiTypes);

    if (source->bidiTypes.count > 0) {
        SBUInteger byteCount;

        ListReserveRange(&bidiTypesBuffer->bidiTypes, 0, source->bidiTypes.count);

        byteCount = source->bidiTypes.count * sizeof(SBBidiType);
        memcpy(bidiTypesBuffer->bidiTypes.items, source->bidiTypes.items, byteCount);
    }
}

SB_INTERNAL void BidiTypesBufferFinalize(BidiTypesBufferRef bidiTypesBuffer)
{
    ListFinalize(&bidiTypesBuffer->bidiTypes);
}

SB_INTERNAL const SBBidiType *BidiTypesBufferGetPtr(BidiTypesBufferRef bidiTypesBuffer, SBUInteger index)
{
    return &bidiTypesBuffer->bidiTypes.items[index];
}

SB_INTERNAL void BidiTypesBufferGetBidiTypes(BidiTypesBufferRef bidiTypesBuffer, SBUInteger index,
    SBUInteger length, SBBidiType *buffer)
{
    const SBBidiType *bidiTypes = &bidiTypesBuffer->bidiTypes.items[index];
    SBUInteger byteCount = length * sizeof(SBBidiType);

    memcpy(buffer, bidiTypes, byteCount);
}

SB_INTERNAL void BidiTypesBufferReplaceRange(BidiTypesBufferRef bidiTypesBuffer, TextBufferRef buffer,
    SBUInteger replaceStart, SBUInteger oldLength, SBUInteger newLength)
{
    if (newLength > oldLength) {
        ListReserveRange(&bidiTypesBuffer->bidiTypes, replaceStart, newLength - oldLength);
    } else {
        ListRemoveRange(&bidiTypesBuffer->bidiTypes, replaceStart, oldLength - newLength);
    }

    DetermineChunkBidiTypes(bidiTypesBuffer, buffer, replaceStart, newLength);
}

#endif
