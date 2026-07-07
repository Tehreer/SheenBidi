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

#ifndef _SB_INTERNAL_TEXT_BUFFER_H
#define _SB_INTERNAL_TEXT_BUFFER_H

#include <API/SBBase.h>

#if SB_TEXT_API_SUPPORTED

#include <SheenBidi/SBCodepointSequence.h>

#include <Core/List.h>

/**
 * Raw code-unit storage for a text object.
 *
 * Holds only the encoding and the code units themselves; has no knowledge of bidirectional
 * properties, script information, or paragraph boundaries.
 */
typedef struct _TextBuffer {
    SBStringEncoding encoding;
    List codeUnits;
} TextBuffer, *TextBufferRef;

/**
 * Initializes an empty text buffer for the given encoding.
 *
 * @param buffer
 *      The text buffer to initialize.
 * @param encoding
 *      The code-unit encoding the buffer will store.
 */
SB_INTERNAL void TextBufferInitialize(TextBufferRef buffer, SBStringEncoding encoding);

/**
 * Replaces an already-initialized text buffer's code units with a deep copy of another buffer's
 * code units. The buffer must already be initialized (via `TextBufferInitialize()`) with the same
 * encoding as `source`.
 *
 * @param buffer
 *      The text buffer to copy into.
 * @param source
 *      The source buffer to copy code units from.
 */
SB_INTERNAL void TextBufferCopyCodeUnits(TextBufferRef buffer, const TextBuffer *source);

/**
 * Finalizes a text buffer and releases its code-unit storage.
 *
 * @param buffer
 *      The text buffer to finalize.
 */
SB_INTERNAL void TextBufferFinalize(TextBufferRef buffer);

/**
 * Returns the number of code units currently stored in the buffer.
 *
 * @param buffer
 *      The text buffer to query.
 */
SB_INTERNAL SBUInteger TextBufferGetLength(TextBufferRef buffer);

/**
 * Returns the maximum number of code units a single code point can occupy in the buffer's
 * encoding.
 *
 * @param buffer
 *      The text buffer to query.
 */
SB_INTERNAL SBUInteger TextBufferGetMaxCodeUnitsPerCodepoint(TextBufferRef buffer);

/**
 * Returns a raw pointer to the code unit at the given index.
 *
 * @param buffer
 *      The text buffer to read from.
 * @param index
 *      The code-unit index to point to.
 */
SB_INTERNAL const void *TextBufferGetCodeUnitsPtr(TextBufferRef buffer, SBUInteger index);

/**
 * Populates a codepoint sequence describing the buffer's entire content.
 *
 * @param buffer
 *      The text buffer to describe.
 * @param sequence
 *      Output codepoint sequence.
 */
SB_INTERNAL void TextBufferGetCodepointSequence(TextBufferRef buffer, SBCodepointSequence *sequence);

/**
 * Copies a range of code units from the buffer into a caller-provided destination.
 *
 * @param buffer
 *      The text buffer to read from.
 * @param index
 *      Start index of the range (in code units).
 * @param length
 *      Number of code units to copy.
 * @param destination
 *      Output buffer; must be large enough for `length` code units.
 */
SB_INTERNAL void TextBufferGetCodeUnits(TextBufferRef buffer, SBUInteger index, SBUInteger length,
    void *destination);

/**
 * Inserts code units at the specified position, shifting existing content right.
 *
 * @param buffer
 *      The text buffer to modify.
 * @param index
 *      Insertion index (in code units).
 * @param codeUnitBuffer
 *      Pointer to code units to insert.
 * @param codeUnitCount
 *      Number of code units to insert.
 */
SB_INTERNAL void TextBufferInsertCodeUnits(TextBufferRef buffer, SBUInteger index,
    const void *codeUnitBuffer, SBUInteger codeUnitCount);

/**
 * Removes a contiguous range of code units, shifting trailing content left.
 *
 * @param buffer
 *      The text buffer to modify.
 * @param index
 *      Start index of the range to delete (in code units).
 * @param length
 *      Number of code units to delete.
 */
SB_INTERNAL void TextBufferDeleteCodeUnits(TextBufferRef buffer, SBUInteger index, SBUInteger length);

/**
 * Replaces a contiguous range of code units with new content.
 *
 * @param buffer
 *      The text buffer to modify.
 * @param index
 *      Start index of the range to replace (in code units).
 * @param oldLength
 *      Length of the range to replace (in code units).
 * @param codeUnitBuffer
 *      Pointer to replacement code units.
 * @param newLength
 *      Number of replacement code units.
 */
SB_INTERNAL void TextBufferReplaceCodeUnits(TextBufferRef buffer, SBUInteger index, SBUInteger oldLength,
    const void *codeUnitBuffer, SBUInteger newLength);

#endif

#endif
