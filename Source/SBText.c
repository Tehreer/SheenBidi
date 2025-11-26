/*
 * Copyright (C) 2025 Muhammad Tayyab Akram
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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "AttributeManager.h"
#include "List.h"
#include "Object.h"
#include "SBAssert.h"
#include "SBAttributeRegistry.h"
#include "SBBase.h"
#include "SBCodepoint.h"
#include "SBCodepointSequence.h"
#include "SBParagraph.h"
#include "SBScriptLocator.h"
#include "SBTextConfig.h"
#include "SBTextIterators.h"
#include "SBText.h"

/* =========================================================================
 * Text Paragraph Implementation
 * ========================================================================= */

 /**
 * Initializes a TextParagraph structure with default values.
 * 
 * @param paragraph
 *      Pointer to the paragraph to initialize.
 */
static void InitializeTextParagraph(TextParagraphRef paragraph)
{
    paragraph->index = SBInvalidIndex;
    paragraph->length = 0;
    paragraph->needsReanalysis = SBTrue;
    paragraph->bidiParagraph = NULL;

    ListInitialize(&paragraph->scripts, sizeof(SBScriptRun));
}

/**
 * Releases resources associated with a TextParagraph structure.
 * 
 * @param paragraph
 *      Pointer to the paragraph structure to finalize.
 */
static void FinalizeTextParagraph(TextParagraphRef paragraph)
{
    SBParagraphRef bidiParagraph = paragraph->bidiParagraph;

    if (bidiParagraph) {
        SBParagraphRelease(bidiParagraph);
    }

    ListFinalize(&paragraph->scripts);
}

/* =========================================================================
 * Text Implementation
 * ========================================================================= */

/**
 * Returns the size in bytes of a single code unit for the given encoding.
 * 
 * @param encoding
 *      The string encoding.
 * @return
 *      Size in bytes of a code unit, or 0 if encoding is invalid.
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

/**
 * Returns the maximum number of code units needed to represent a single code point in the given
 * encoding.
 * 
 * @param text
 *      The text object.
 * @return
 *      Maximum code units per code point, or 0 if encoding is invalid.
 */
static SBUInteger GetMaxCodeUnitsPerCodepoint(SBTextRef text)
{
    switch (text->encoding) {
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

/**
 * Finalizes all paragraphs in the text object by releasing their resources.
 * 
 * @param text
 *      The text object whose paragraphs will be finalized.
 */
static void FinalizeAllParagraphs(SBTextRef text)
{
    SBUInteger paragraphIndex;

    for (paragraphIndex = 0; paragraphIndex < text->paragraphs.count; paragraphIndex++) {
        FinalizeTextParagraph(ListGetRef(&text->paragraphs, paragraphIndex));
    }
}

/**
 * Comparison function for binary search to locate a paragraph containing a specific code unit. Used
 * by bsearch() to find the paragraph that contains a given code unit index.
 */
static int ParagraphIndexComparison(const void *key, const void *element) {
    const SBUInteger *codeUnitIndex = key;
    const TextParagraph *paragraph = element;
    SBUInteger paragraphStart;
    SBUInteger paragraphEnd;

    paragraphStart = paragraph->index;
    paragraphEnd = paragraphStart + paragraph->length;

    if (*codeUnitIndex < paragraphStart) {
        return -1;
    }
    if (*codeUnitIndex >= paragraphEnd) {
        return 1;
    }

    return 0;
}

SB_INTERNAL SBUInteger SBTextGetCodeUnitParagraphIndex(SBTextRef text, SBUInteger codeUnitIndex)
{
    TextParagraph *array = text->paragraphs.items;
    SBUInteger count = text->paragraphs.count;
    void *item = NULL;

    if (array) {
        item = bsearch(&codeUnitIndex, array, count, sizeof(TextParagraph), ParagraphIndexComparison);
    }

    if (item) {
        return (SBUInteger)((TextParagraph *)item - array);
    }

    return SBInvalidIndex;
}

SB_INTERNAL void SBTextGetBoundaryParagraphs(SBTextRef text,
    SBUInteger rangeStart, SBUInteger rangeEnd,
    TextParagraphRef *firstParagraph, TextParagraphRef *lastParagraph)
{
    SBUInteger codeUnitCount = text->codeUnits.count;

    SBAssert(firstParagraph && lastParagraph);

    *firstParagraph = NULL;
    *lastParagraph = NULL;

    /* Find the first paragraph intersecting the range */
    if (rangeStart < codeUnitCount) {
        SBUInteger paragraphIndex;
        SBUInteger paragraphStart;
        SBUInteger paragraphEnd;

        paragraphIndex = SBTextGetCodeUnitParagraphIndex(text, rangeStart);
        *firstParagraph = ListGetRef(&text->paragraphs, paragraphIndex);

        paragraphStart = (*firstParagraph)->index;
        paragraphEnd = paragraphStart + (*firstParagraph)->length;

        /* If the range doesn't extend beyond the first paragraph, they're the same */
        if (paragraphEnd >= rangeEnd) {
            *lastParagraph = *firstParagraph;
            return;
        }
    }

    /* Find the last paragraph if it's different from the first */
    if (rangeEnd <= codeUnitCount) {
        SBUInteger paragraphIndex;

        paragraphIndex = SBTextGetCodeUnitParagraphIndex(text, rangeEnd - 1);
        *lastParagraph = ListGetRef(&text->paragraphs, paragraphIndex);
    }
}

SBTextRef SBTextCreate(const void *string, SBUInteger length, SBStringEncoding encoding,
    SBTextConfigRef config)
{
    SBMutableTextRef text = SBTextCreateMutable(encoding, config);

    if (text) {
        SBTextAppendCodeUnits(text, string, length);
        text->isMutable = SBFalse;
    }

    return text;
}

SBTextRef SBTextCreateCopy(SBTextRef text)
{
    SBMutableTextRef copy = SBTextCreateMutableCopy(text);

    if (copy) {
        copy->isMutable = SBFalse;
    }

    return copy;
}

SBStringEncoding SBTextGetEncoding(SBTextRef text)
{
    return text->encoding;
}

SBAttributeRegistryRef SBTextGetAttributeRegistry(SBTextRef text)
{
    return text->attributeRegistry;
}

SBUInteger SBTextGetLength(SBTextRef text)
{
    return text->codeUnits.count;
}

void SBTextGetCodeUnits(SBTextRef text, SBUInteger index, SBUInteger length, void *buffer)
{
    SBBoolean isRangeValid = SBUIntegerVerifyRange(text->codeUnits.count, index, length);
    SBUInteger byteCount;
    const void *source;

    SBAssert(isRangeValid);

    byteCount = length * text->codeUnits.itemSize;
    source = ListGetPtr(&text->codeUnits, index);

    memcpy(buffer, source, byteCount);
}

void SBTextGetBidiTypes(SBTextRef text, SBUInteger index, SBUInteger length, SBBidiType *buffer)
{
    SBBoolean isRangeValid = SBUIntegerVerifyRange(text->codeUnits.count, index, length);
    const SBBidiType *bidiTypes;
    SBUInteger byteCount;

    SBAssert(isRangeValid);

    bidiTypes = &text->bidiTypes.items[index];
    byteCount = length * sizeof(SBBidiType);

    memcpy(buffer, bidiTypes, byteCount);
}

void SBTextGetScripts(SBTextRef text, SBUInteger index, SBUInteger length, SBScript *buffer)
{
    SBBoolean isRangeValid = SBUIntegerVerifyRange(text->codeUnits.count, index, length);
    SBUInteger rangeStart;
    SBUInteger rangeEnd;
    SBUInteger paragraphIndex;

    SBAssert(isRangeValid && !text->isEditing);

    rangeStart = index;
    rangeEnd = index + length;
    paragraphIndex = SBTextGetCodeUnitParagraphIndex(text, rangeStart);

    while (rangeStart < rangeEnd) {
        const TextParagraph *textParagraph = ListGetRef(&text->paragraphs, paragraphIndex);
        SBUInteger copyStart = textParagraph->index;
        SBUInteger copyEnd = copyStart + textParagraph->length;
        const SBScript *scriptArray;
        SBUInteger scriptCount;
        SBUInteger byteCount;

        /* Clamp copy range to requested range */
        if (copyStart < rangeStart) {
            copyStart = rangeStart;
        }
        if (copyEnd > rangeEnd) {
            copyEnd = rangeEnd;
        }

        scriptArray = ListGetRef(&textParagraph->scripts, copyStart - textParagraph->index);
        scriptCount = copyEnd - copyStart;
        byteCount = scriptCount * sizeof(SBScript);

        memcpy(buffer, scriptArray, byteCount);

        buffer += scriptCount;
        rangeStart = copyEnd;
        paragraphIndex += 1;
    }
}

void SBTextGetResolvedLevels(SBTextRef text, SBUInteger index, SBUInteger length, SBLevel *buffer)
{
    SBBoolean isRangeValid = SBUIntegerVerifyRange(text->codeUnits.count, index, length);
    SBUInteger rangeStart;
    SBUInteger rangeEnd;
    SBUInteger paragraphIndex;

    SBAssert(isRangeValid && !text->isEditing);

    rangeStart = index;
    rangeEnd = index + length;
    paragraphIndex = SBTextGetCodeUnitParagraphIndex(text, index);

    while (rangeStart < rangeEnd) {
        const TextParagraph *textParagraph = ListGetRef(&text->paragraphs, paragraphIndex);
        SBUInteger copyStart = textParagraph->index;
        SBUInteger copyEnd = copyStart + textParagraph->length;
        SBParagraphRef bidiParagraph;
        const SBLevel *levelArray;
        SBUInteger levelCount;
        SBUInteger byteCount;

        /* Clamp copy range to requested range */
        if (copyStart < rangeStart) {
            copyStart = rangeStart;
        }
        if (copyEnd > rangeEnd) {
            copyEnd = rangeEnd;
        }

        bidiParagraph = textParagraph->bidiParagraph;
        levelArray = &bidiParagraph->fixedLevels[copyStart - bidiParagraph->offset];
        levelCount = copyEnd - copyStart;
        byteCount = levelCount * sizeof(SBLevel);

        memcpy(buffer, levelArray, byteCount);

        buffer += levelCount;
        rangeStart = copyEnd;
        paragraphIndex += 1;
    }
}

void SBTextGetCodeUnitParagraphInfo(SBTextRef text, SBUInteger index,
    SBParagraphInfo *paragraphInfo)
{
    SBBoolean isValidIndex = index < text->codeUnits.count;
    SBUInteger paragraphIndex;
    const TextParagraph *textParagraph;
    SBParagraphRef bidiParagraph;

    SBAssert(isValidIndex && !text->isEditing);

    paragraphIndex = SBTextGetCodeUnitParagraphIndex(text, index);
    textParagraph = ListGetRef(&text->paragraphs, paragraphIndex);
    bidiParagraph = textParagraph->bidiParagraph;

    paragraphInfo->index = textParagraph->index;
    paragraphInfo->length = textParagraph->length;
    paragraphInfo->baseLevel = bidiParagraph->baseLevel;
}

SBParagraphIteratorRef SBTextCreateParagraphIterator(SBTextRef text)
{
    return SBParagraphIteratorCreate(text);
}

SBLogicalRunIteratorRef SBTextCreateLogicalRunIterator(SBTextRef text)
{
    return SBLogicalRunIteratorCreate(text);
}

SBScriptRunIteratorRef SBTextCreateScriptRunIterator(SBTextRef text)
{
    return SBScriptRunIteratorCreate(text);
}

SBAttributeRunIteratorRef SBTextCreateAttributeRunIterator(SBTextRef text)
{
    return SBAttributeRunIteratorCreate(text);
}

SBTextRef SBTextRetain(SBTextRef text)
{
    return ObjectRetain((ObjectRef)text);
}

void SBTextRelease(SBTextRef text)
{
    ObjectRelease((ObjectRef)text);
}

/* =========================================================================
 * Mutable Text Implementation
 * ========================================================================= */

static void DetermineChunkBidiTypes(SBMutableTextRef text, SBUInteger index, SBUInteger length)
{
    SBUInteger codeUnitCount = text->codeUnits.count;

    if (codeUnitCount > 0) {
        SBUInteger startIndex = index;
        SBUInteger endIndex = startIndex + length;
        SBStringEncoding encoding = text->encoding;
        const void *buffer = text->codeUnits.data;
        SBUInteger surround;
        SBCodepointSequence sequence;

        surround = GetMaxCodeUnitsPerCodepoint(text);

        startIndex = (startIndex >= surround ? startIndex - surround : 0);
        endIndex = ((endIndex + surround) <= codeUnitCount ? endIndex + surround : codeUnitCount);
        endIndex -= 1;

        /* Align to code point boundaries */
        SBCodepointSkipToStart(buffer, codeUnitCount, encoding, &startIndex);
        SBCodepointSkipToEnd(buffer, codeUnitCount, encoding, &endIndex);

        sequence.stringEncoding = encoding;
        sequence.stringBuffer = SBCodepointGetBufferOffset(buffer, encoding, startIndex);
        sequence.stringLength = endIndex - startIndex;

        SBCodepointSequenceDetermineBidiTypes(&sequence, &text->bidiTypes.items[startIndex]);
    }
}

static SBBoolean InsertBidiTypes(SBMutableTextRef text, SBUInteger index, SBUInteger length)
{
    SBBoolean succeeded = SBFalse;

    if (ListReserveRange(&text->bidiTypes, index, length)) {
        DetermineChunkBidiTypes(text, index, length);
        succeeded = SBTrue;
    }

    return succeeded;
}

static void RemoveBidiTypes(SBMutableTextRef text, SBUInteger index, SBUInteger length)
{
    ListRemoveRange(&text->bidiTypes, index, length);
    DetermineChunkBidiTypes(text, index, 0);
}

static TextParagraphRef InsertEmptyParagraph(SBMutableTextRef text, SBUInteger listIndex)
{
    SBBoolean succeeded;
    TextParagraph paragraph;

    InitializeTextParagraph(&paragraph);
    succeeded = ListInsert(&text->paragraphs, listIndex, &paragraph);

    return (succeeded ? ListGetRef(&text->paragraphs, listIndex) : NULL);
}

static void RemoveParagraphRange(SBMutableTextRef text, SBUInteger index, SBUInteger length)
{
    SBUInteger endIndex = index + length;
    SBUInteger paragraphIndex;

    /* Finalize each paragraph's resources */
    for (paragraphIndex = index; paragraphIndex < endIndex; paragraphIndex++) {
        TextParagraphRef paragraph = ListGetRef(&text->paragraphs, paragraphIndex);
        FinalizeTextParagraph(paragraph);
    }

    ListRemoveRange(&text->paragraphs, index, length);
}

/**
 * Adjusts the start index of all paragraphs from a given position onward by a delta.
 * Used when text is inserted or deleted to shift paragraph boundaries.
 * 
 * @param text
 *      Mutable text object.
 * @param listIndex
 *      Starting list position (inclusive).
 * @param indexDelta
 *      Amount to add to each paragraph's index (can be negative).
 */
static void ShiftParagraphRanges(SBMutableTextRef text, SBUInteger listIndex, SBInteger indexDelta) {
    while (listIndex < text->paragraphs.count) {
        TextParagraphRef paragraph = ListGetRef(&text->paragraphs, listIndex);
        paragraph->index += indexDelta;
        listIndex += 1;
    }
}

/**
 * Given a paragraph index, re-analyze the combined area formed by
 * firstParagraph .. nextParagraph and decide whether they should be merged, or the next paragraph
 * should be adjusted.
 *
 * This function:
 *  - recomputes the paragraph boundary starting at firstParagraph->index across the bytes up to
 *    (next.index + next.length) exclusive,
 *  - if the recomputed first paragraph covers the whole window, removes the next paragraph;
 *    otherwise updates nextParagraph->index/length accordingly.
 */
static void MergeParagraphsIfNeeded(SBMutableTextRef text, SBUInteger listIndex)
{
    if (listIndex < text->paragraphs.count - 1) {
        TextParagraph *firstParagraph = ListGetRef(&text->paragraphs, listIndex);
        TextParagraph *nextParagraph  = ListGetRef(&text->paragraphs, listIndex + 1);
        SBUInteger windowStart;
        SBUInteger windowEnd;
        SBCodepointSequence sequence;

        windowStart = firstParagraph->index;
        windowEnd = nextParagraph->index + nextParagraph->length;

        /* Clamp window to text bounds */
        if (windowEnd > text->codeUnits.count) {
            windowEnd = text->codeUnits.count;
        }

        sequence.stringEncoding = text->encoding;
        sequence.stringBuffer = SBCodepointGetBufferOffset(
            text->codeUnits.data, text->encoding, windowStart
        );
        sequence.stringLength = windowEnd - windowStart;

        /* Recompute paragraph boundary */
        SBCodepointSequenceGetParagraphBoundary(
            &sequence, &text->bidiTypes.items[windowStart],
            0, sequence.stringLength, &firstParagraph->length, NULL
        );

        if (firstParagraph->length == sequence.stringLength) {
            /* Entire span is one paragraph; remove the next one. */
            RemoveParagraphRange(text, listIndex + 1, 1);
        } else {
            /* Otherwise adjust next paragraph's index and length to the tail portion */
            nextParagraph->index = firstParagraph->index + firstParagraph->length;
            nextParagraph->length = sequence.stringLength - firstParagraph->length;
        }
    }
}

/**
 * Updates paragraph list for an insertion at `index` of `length` code units.
 *
 * Strategy:
 *  - Find the paragraph containing the insertion index (or choose the last paragraph if not found).
 *  - Shift paragraphs after insertion right by length.
 *  - Re-scan the affected area starting from the paragraph start (so CRLF split is honored).
 *  - Insert new paragraph entries when new boundaries are discovered.
 */
static SBBoolean UpdateParagraphsForTextInsertion(SBMutableTextRef text, SBUInteger index, SBUInteger length)
{
    TextParagraphRef paragraph = NULL;
    const SBBidiType *bidiTypes;
    SBCodepointSequence sequence;
    SBUInteger listIndex;
    SBUInteger scanIndex;
    SBUInteger remaining;

    /* Locate the paragraph that contains the insertion point */
    listIndex = SBTextGetCodeUnitParagraphIndex(text, index);

    if (listIndex == SBInvalidIndex) {
        listIndex = text->paragraphs.count;

        /* Check if we should extend the last paragraph or start a new one */
        if (text->codeUnits.count > 0) {
            SBUInteger lastIndex = text->codeUnits.count - 1;
            SBBidiType bidiType;

            SBCodepointSkipToStart(text->codeUnits.data, text->codeUnits.count,
                text->encoding, &lastIndex);

            bidiType = ListGetVal(&text->bidiTypes, lastIndex);

            if (bidiType != SBBidiTypeB) {
                listIndex -= 1;
            }
        }
    }

    if (listIndex < text->paragraphs.count) {
        /* Shift subsequent paragraph ranges to the right by insertion length */
        ShiftParagraphRanges(text, listIndex + 1, length);

        paragraph = ListGetRef(&text->paragraphs, listIndex);
        paragraph->length += length;

        /* Re-analyze starting from the start of the paragraph to correctly handle separators */
        length += index - paragraph->index;
        index = paragraph->index;

        /* Ensure full paragraph is analyzed */
        if (length < paragraph->length) {
            length = paragraph->length;
        }
    } else {
        listIndex = text->paragraphs.count;
    }

    /* Set up code point sequence for analysis */
    sequence.stringEncoding = text->encoding;
    sequence.stringBuffer = SBCodepointGetBufferOffset(text->codeUnits.data, text->encoding, index);
    sequence.stringLength = length;

    bidiTypes = &text->bidiTypes.items[index];

    scanIndex = 0;
    remaining = sequence.stringLength;

    /* Iterate and write paragraph entries */
    while (remaining > 0) {
        SBUInteger boundary;

        /* Compute boundary within remaining region */
        SBCodepointSequenceGetParagraphBoundary(
            &sequence, bidiTypes, scanIndex, remaining, &boundary, NULL);

        if (!paragraph) {
            paragraph = InsertEmptyParagraph(text, listIndex);
        }
        if (paragraph) {
            paragraph->index = index + scanIndex;
            paragraph->length = boundary;
            paragraph->needsReanalysis = SBTrue;
            paragraph = NULL;
        } else {
            return SBFalse;
        }

        /* Advance */
        scanIndex += boundary;
        remaining -= boundary;
        listIndex += 1;  
    }

    return SBTrue;
}

static void UpdateParagraphsForTextRemoval(SBMutableTextRef text, SBUInteger index, SBUInteger length)
{
    SBUInteger rangeEnd = index + length;

    if (text->codeUnits.count > 0) {
        /* Locate first and last paragraphs affected. */
        SBUInteger firstIndex = SBTextGetCodeUnitParagraphIndex(text, index);
        SBUInteger lastIndex = SBTextGetCodeUnitParagraphIndex(text, rangeEnd - 1);

        if (firstIndex == lastIndex) {
            TextParagraphRef paragraph = ListGetRef(&text->paragraphs, firstIndex);

            /* Exclude removed range */
            paragraph->length -= length;
            paragraph->needsReanalysis = SBTrue;
        } else {
            TextParagraphRef firstParagraph = ListGetRef(&text->paragraphs, firstIndex);
            TextParagraphRef lastParagraph = ListGetRef(&text->paragraphs, lastIndex);

            /* Trim the first paragraph to the portion before the removal start */
            firstParagraph->length = index - firstParagraph->index;
            firstParagraph->needsReanalysis = SBTrue;

            /* Adjust the last paragraph */
            if (rangeEnd < lastParagraph->index + lastParagraph->length) {
                SBUInteger shift = rangeEnd - lastParagraph->index;
                lastParagraph->index = index;
                lastParagraph->length -= shift;
                lastParagraph->needsReanalysis = SBTrue;
            }

            /* Remove fully-covered middle paragraphs and update the last index */
            if (lastIndex > firstIndex + 1) {
                SBUInteger removeCount = lastIndex - firstIndex - 1;
                RemoveParagraphRange(text, firstIndex + 1, removeCount);
                lastIndex -= removeCount;
            }
        }

        /* Shift all following paragraphs to the left */
        ShiftParagraphRanges(text, lastIndex + 1, -length);

        /* Merge if paragraph separator was removed */
        MergeParagraphsIfNeeded(text, firstIndex);
    } else {
        /* Remove all paragraphs */
        RemoveParagraphRange(text, 0, text->paragraphs.count);
    }
}

static SBBoolean GenerateBidiParagraph(SBMutableTextRef text, TextParagraphRef paragraph)
{
    SBCodepointSequence codepointSequence;
    const SBBidiType *bidiTypes;

    codepointSequence.stringEncoding = text->encoding;
    codepointSequence.stringBuffer = text->codeUnits.data;
    codepointSequence.stringLength = text->codeUnits.count;

    bidiTypes = text->bidiTypes.items;

    if (paragraph->bidiParagraph) {
        /* Release old bidi paragraph */
        SBParagraphRelease(paragraph->bidiParagraph);
        paragraph->bidiParagraph = NULL;
    }

    paragraph->bidiParagraph = SBParagraphCreateWithCodepointSequence(
        &codepointSequence, bidiTypes, paragraph->index, paragraph->length, text->baseLevel);

    return (paragraph->bidiParagraph != NULL);
}

static SBBoolean PopulateScripts(SBMutableTextRef text, TextParagraphRef paragraph)
{
    SBBoolean succeeded;
    SBScriptLocatorRef scriptLocator;
    SBCodepointSequence codepointSequence;

    scriptLocator = text->scriptLocator;

    codepointSequence.stringEncoding = text->encoding;
    codepointSequence.stringBuffer = ListGetPtr(&text->codeUnits, paragraph->index);
    codepointSequence.stringLength = paragraph->length;

    ListRemoveAll(&paragraph->scripts);
    succeeded = ListReserveRange(&paragraph->scripts, 0, paragraph->length);

    if (succeeded) {
        const SBScriptAgent *scriptAgent = &scriptLocator->agent;

        SBScriptLocatorLoadCodepoints(scriptLocator, &codepointSequence);

        while (SBScriptLocatorMoveNext(scriptLocator)) {
            SBUInteger runStart = scriptAgent->offset;
            SBUInteger runEnd = runStart + scriptAgent->length;
            SBScript runScript = scriptAgent->script;

            while (runStart < runEnd) {
                ListSetVal(&paragraph->scripts, runStart, runScript);
                runStart += 1;
            }
        }
    }

    return succeeded;
}

/**
 * Analyzes all paragraphs marked as needing reanalysis.
 * Generates bidirectional properties and script information.
 */
static SBBoolean AnalyzeDirtyParagraphs(SBMutableTextRef text)
{
    SBBoolean succeeded = SBTrue;
    SBUInteger paragraphIndex;

    for (paragraphIndex = 0; paragraphIndex < text->paragraphs.count; paragraphIndex++) {
        TextParagraphRef paragraph = ListGetRef(&text->paragraphs, paragraphIndex);

        if (paragraph->needsReanalysis) {
            succeeded = GenerateBidiParagraph(text, paragraph);

            if (succeeded) {
                succeeded = PopulateScripts(text, paragraph);
            }

            paragraph->needsReanalysis = SBFalse;
        }

        if (!succeeded) {
            break;
        }
    }

    return succeeded;
}

/**
 * Cleanup callback for mutable text objects; releases all owned resources.
 */
static void FinalizeMutableText(ObjectRef object)
{
    SBMutableTextRef text = object;

    AttributeManagerFinalize(&text->attributeManager);
    FinalizeAllParagraphs(text);

    ListFinalize(&text->codeUnits);
    ListFinalize(&text->bidiTypes);
    ListFinalize(&text->paragraphs);

    if (text->scriptLocator) {
        SBScriptLocatorRelease(text->scriptLocator);
    }
    if (text->attributeRegistry) {
        SBAttributeRegistryRelease(text->attributeRegistry);
    }
}

SB_INTERNAL SBMutableTextRef SBTextCreateMutableWithParameters(SBStringEncoding encoding,
    SBAttributeRegistryRef attributeRegistry, SBLevel baseLevel)
{
    const SBUInteger size = sizeof(SBText);
    void *pointer = NULL;
    SBMutableTextRef text;

    text = ObjectCreate(&size, 1, &pointer, FinalizeMutableText);

    if (text) {
        if (attributeRegistry) {
            attributeRegistry = SBAttributeRegistryRetain(attributeRegistry);
        }

        text->encoding = encoding;
        text->isMutable = SBTrue;
        text->baseLevel = baseLevel;
        text->isEditing = SBFalse;
        text->scriptLocator = SBScriptLocatorCreate();
        text->attributeRegistry = attributeRegistry;

        AttributeManagerInitialize(&text->attributeManager, text, attributeRegistry);
        ListInitialize(&text->codeUnits, GetCodeUnitSize(encoding));
        ListInitialize(&text->bidiTypes, sizeof(SBBidiType));
        ListInitialize(&text->paragraphs, sizeof(TextParagraph));
    }

    return text;
}

SBMutableTextRef SBTextCreateMutable(SBStringEncoding encoding, SBTextConfigRef config)
{
    SBMutableTextRef text = SBTextCreateMutableWithParameters(encoding,
        config->attributeRegistry, config->baseLevel);

    if (text) {
        /* TODO: Apply default attributes */
    }

    return text;
}

SBMutableTextRef SBTextCreateMutableCopy(SBTextRef text)
{
    SBMutableTextRef copy = SBTextCreateMutableWithParameters(text->encoding,
        text->attributeRegistry, text->baseLevel);

    if (copy) {
        SBBoolean succeeded;

        /* Copy code units */
        succeeded = ListReserveRange(&copy->codeUnits, 0, text->codeUnits.count);
        if (succeeded) {
            SBUInteger byteCount = text->codeUnits.count * text->codeUnits.itemSize;
            memcpy(copy->codeUnits.data, text->codeUnits.data, byteCount);
        }

        /* Copy bidi types */
        if (succeeded) {
            succeeded = ListReserveRange(&copy->bidiTypes, 0, text->bidiTypes.count);
            if (succeeded) {
                SBUInteger byteCount = text->bidiTypes.count * sizeof(SBBidiType);
                memcpy(copy->bidiTypes.items, text->bidiTypes.items, byteCount);
            }
        }

        /* Copy paragraphs */
        if (succeeded) {
            SBUInteger paragraphCount = text->paragraphs.count;
            SBUInteger paragraphIndex;

            succeeded = ListReserveRange(&copy->paragraphs, 0, paragraphCount);

            if (succeeded) {
                for (paragraphIndex = 0; paragraphIndex < paragraphCount; paragraphIndex++) {
                    TextParagraphRef source = ListGetRef(&text->paragraphs, paragraphIndex);
                    TextParagraphRef destination = ListGetRef(&copy->paragraphs, paragraphIndex);

                    destination->index = source->index;
                    destination->length = source->length;
                    ListInitialize(&destination->scripts, sizeof(SBScript));

                    if (source->needsReanalysis) {
                        destination->needsReanalysis = SBTrue;
                        destination->bidiParagraph = NULL;
                    } else {
                        SBUInteger scriptCount = source->scripts.count;
                        SBUInteger byteCount = scriptCount * sizeof(SBScript);

                        destination->needsReanalysis = SBFalse;
                        destination->bidiParagraph = SBParagraphRetain(source->bidiParagraph);

                        ListReserveRange(&destination->scripts, 0, scriptCount);
                        memcpy(destination->scripts.items, source->scripts.items, byteCount);
                    }
                }

                succeeded = AnalyzeDirtyParagraphs(copy);
            }
        }

        /* Copy attributes */
        AttributeManagerCopyAttributes(&copy->attributeManager, &text->attributeManager);

        /* Cleanup if unsuccessful */
        if (!succeeded) {
            SBTextRelease(copy);
            copy = NULL;
        }
    }

    return copy;
}

void SBTextBeginEditing(SBMutableTextRef text)
{
    SBAssert(text->isMutable);

    text->isEditing = SBTrue;
}

SBBoolean SBTextEndEditing(SBMutableTextRef text)
{
    SBBoolean succeeded;

    SBAssert(text->isMutable);

    succeeded = AnalyzeDirtyParagraphs(text);
    text->isEditing = SBFalse;

    return succeeded;
}

SBBoolean SBTextAppendCodeUnits(SBMutableTextRef text,
    const void *codeUnitBuffer, SBUInteger codeUnitCount)
{
    SBAssert(text->isMutable);

    return SBTextInsertCodeUnits(text, text->codeUnits.count, codeUnitBuffer, codeUnitCount);
}

SBBoolean SBTextInsertCodeUnits(SBMutableTextRef text, SBUInteger index,
    const void *codeUnitBuffer, SBUInteger codeUnitCount)
{
    SBBoolean succeeded = SBTrue;

    SBAssert(text->isMutable && index <= text->codeUnits.count);

    if (codeUnitCount > 0) {
        succeeded = SBFalse;

        /* Reserve space in code units */
        if (ListReserveRange(&text->codeUnits, index, codeUnitCount)) {
            SBUInteger byteCount = codeUnitCount * text->codeUnits.itemSize;
            void *destination = ListGetPtr(&text->codeUnits, index);

            memcpy(destination, codeUnitBuffer, byteCount);
            succeeded = SBTrue;
        }

        /* Insert bidi types */
        if (succeeded) {
            succeeded = InsertBidiTypes(text, index, codeUnitCount);
        }

        /* Reserve attribute manager space */
        AttributeManagerReserveRange(&text->attributeManager, index, codeUnitCount);

        /* Update paragraph structures */
        if (succeeded) {
            succeeded = UpdateParagraphsForTextInsertion(text, index, codeUnitCount);
        }

        if (succeeded) {
            /* Perform immediate analysis if not in batch editing mode */
            if (!text->isEditing) {
                succeeded = AnalyzeDirtyParagraphs(text);
            }
        }
    }

    return succeeded;
}

SBBoolean SBTextDeleteCodeUnits(SBMutableTextRef text, SBUInteger index, SBUInteger length)
{
    SBUInteger rangeEnd = index + length;
    SBBoolean isRangeValid = (rangeEnd <= text->codeUnits.count && index <= rangeEnd);
    SBBoolean succeeded = SBTrue;

    SBAssert(text->isMutable && isRangeValid);

    if (length > 0) {
        /* Remove code units */
        ListRemoveRange(&text->codeUnits, index, length);

        /* Remove bidi types */
        RemoveBidiTypes(text, index, length);

        /* Update paragraph structures */
        UpdateParagraphsForTextRemoval(text, index, length);

        /* Remove from attribute manager */
        AttributeManagerRemoveRange(&text->attributeManager, index, length);

        if (!text->isEditing) {
            /* Perform immediate analysis if not in batch editing mode */
            succeeded = AnalyzeDirtyParagraphs(text);
        }
    }

    return succeeded;
}

SBBoolean SBTextSetCodeUnits(SBMutableTextRef text,
    const void *codeUnitBuffer, SBUInteger codeUnitCount)
{
    SBAssert(text->isMutable);

    return SBTextReplaceCodeUnits(text, 0, text->codeUnits.count, codeUnitBuffer, codeUnitCount);
}

SBBoolean SBTextReplaceCodeUnits(SBMutableTextRef text, SBUInteger index, SBUInteger length,
    const void *codeUnitBuffer, SBUInteger codeUnitCount)
{
    SBUInteger rangeEnd = index + length;
    SBBoolean isRangeValid = (rangeEnd <= text->codeUnits.count && index <= rangeEnd);

    SBAssert(text->isMutable && isRangeValid);

    if (length > 0) {
        /* Remove code units */
        ListRemoveRange(&text->codeUnits, index, length);

        /* Remove bidi types */
        RemoveBidiTypes(text, index, length);

        /* Update paragraph structures */
        UpdateParagraphsForTextRemoval(text, index, length);
    }

    if (codeUnitCount > 0) {
        SBUInteger byteCount;
        void *destination;

        /* Reserve space in code units */
        ListReserveRange(&text->codeUnits, index, codeUnitCount);

        byteCount = codeUnitCount * text->codeUnits.itemSize;
        destination = ListGetPtr(&text->codeUnits, index);
        memcpy(destination, codeUnitBuffer, byteCount);

        /* Insert bidi types */
        InsertBidiTypes(text, index, codeUnitCount);

        /* Update paragraph structures */
        UpdateParagraphsForTextInsertion(text, index, codeUnitCount);
    }

    AttributeManagerReplaceRange(&text->attributeManager, index, length, codeUnitCount);

    if (!text->isEditing) {
        SBBoolean needsReanalysis = (length > 0 || codeUnitCount > 0);

        if (needsReanalysis) {
            /* Perform immediate analysis if not in batch editing mode */
            AnalyzeDirtyParagraphs(text);
        }
    }

    return SBTrue;
}

SBBoolean SBTextSetAttribute(SBMutableTextRef text, SBUInteger index, SBUInteger length,
    SBAttributeID attributeID, const void *attributeValue)
{
    SBUInteger rangeEnd = index + length;
    SBBoolean isRangeValid = (rangeEnd <= text->codeUnits.count && index <= rangeEnd);

    SBAssert(text->isMutable && isRangeValid);

    if (length > 0) {
        AttributeManagerSetAttribute(&text->attributeManager,
            index, length, attributeID, attributeValue);
    }

    return SBTrue;
}

SBBoolean SBTextRemoveAttribute(SBMutableTextRef text, SBUInteger index, SBUInteger length,
    SBAttributeID attributeID)
{
    SBUInteger rangeEnd = index + length;
    SBBoolean isRangeValid = (rangeEnd <= text->codeUnits.count && index <= rangeEnd);

    SBAssert(text->isMutable && isRangeValid);

    if (length > 0) {
        AttributeManagerRemoveAttribute(&text->attributeManager, index, length, attributeID);
    }

    return SBTrue;
}
