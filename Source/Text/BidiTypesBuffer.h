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

#ifndef _SB_INTERNAL_BIDI_TYPES_BUFFER_H
#define _SB_INTERNAL_BIDI_TYPES_BUFFER_H

#include <API/SBBase.h>

#if SB_TEXT_API_SUPPORTED

#include <Core/List.h>
#include <Text/TextBuffer.h>

/**
 * Derived bidirectional types for every code unit of a `TextBuffer`.
 *
 * Unlike paragraph/script analysis, bidi types are recomputed eagerly on every edit (never
 * deferred), since paragraph boundary detection needs an up-to-date array as an input.
 */
typedef struct _BidiTypesBuffer {
    LIST(SBBidiType) bidiTypes;
} BidiTypesBuffer, *BidiTypesBufferRef;

/**
 * Initializes an empty bidi-types buffer.
 *
 * @param bidiTypesBuffer
 *      The bidi-types buffer to initialize.
 */
SB_INTERNAL void BidiTypesBufferInitialize(BidiTypesBufferRef bidiTypesBuffer);

/**
 * Replaces an already-initialized bidi-types buffer's content with a deep copy of another
 * buffer's bidi types. The buffer must already be initialized (via `BidiTypesBufferInitialize()`).
 *
 * @param bidiTypesBuffer
 *      The bidi-types buffer to copy into.
 * @param source
 *      The source buffer to copy bidi types from.
 */
SB_INTERNAL void BidiTypesBufferCopyBidiTypes(BidiTypesBufferRef bidiTypesBuffer,
    const BidiTypesBuffer *source);

/**
 * Finalizes a bidi-types buffer and releases its storage.
 *
 * @param bidiTypesBuffer
 *      The bidi-types buffer to finalize.
 */
SB_INTERNAL void BidiTypesBufferFinalize(BidiTypesBufferRef bidiTypesBuffer);

/**
 * Returns a raw pointer to the bidi type at the given index.
 *
 * @param bidiTypesBuffer
 *      The bidi-types buffer to read from.
 * @param index
 *      The code-unit index to point to.
 */
SB_INTERNAL const SBBidiType *BidiTypesBufferGetPtr(BidiTypesBufferRef bidiTypesBuffer, SBUInteger index);

/**
 * Copies bidirectional types for a code-unit range.
 *
 * @param bidiTypesBuffer
 *      The bidi-types buffer to read from.
 * @param index
 *      Start index (in code units).
 * @param length
 *      Number of code units to copy bidi types for.
 * @param buffer
 *      Output array of `SBBidiType` with `length` entries.
 */
SB_INTERNAL void BidiTypesBufferGetBidiTypes(BidiTypesBufferRef bidiTypesBuffer, SBUInteger index,
    SBUInteger length, SBBidiType *buffer);

/**
 * Re-derives bidi types for a code-unit range that was just replaced in `buffer`, expanding the
 * recomputation window to cover surrounding code points that could be affected.
 *
 * @param bidiTypesBuffer
 *      The bidi-types buffer to update.
 * @param buffer
 *      The buffer the code units were replaced in (already updated by the caller).
 * @param replaceStart
 *      Start index of the replaced range (in code units).
 * @param oldLength
 *      Length of the range before replacement.
 * @param newLength
 *      Length of the range after replacement.
 */
SB_INTERNAL void BidiTypesBufferReplaceRange(BidiTypesBufferRef bidiTypesBuffer, TextBufferRef buffer,
    SBUInteger replaceStart, SBUInteger oldLength, SBUInteger newLength);

#endif

#endif
