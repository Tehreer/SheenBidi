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
#include <stdlib.h>
#include <string.h>

#include <SheenBidi/SBCodepointSequence.h>
#include <SheenBidi/SBParagraph.h>
#include <SheenBidi/SBScriptLocator.h>

#include <API/SBAssert.h>
#include <API/SBCodepointSequence.h>
#include <API/SBParagraph.h>
#include <API/SBScriptLocator.h>
#include <Core/List.h>
#include <Text/BidiTypesBuffer.h>
#include <Text/TextBuffer.h>

#include "TextAnalysis.h"

/* =========================================================================
 * Copy-array helper (shared by GetScripts/GetResolvedLevels)
 * ========================================================================= */

enum {
    CopyArrayScripts = 0,
    CopyArrayLevels = 1
};
typedef SBUInt8 CopyArrayKind;

/* =========================================================================
 * Text Paragraph Implementation
 * ========================================================================= */

static void InitializeTextParagraph(TextParagraphRef paragraph)
{
    paragraph->index = SBInvalidIndex;
    paragraph->length = 0;
    paragraph->needsReanalysis = SBTrue;
    paragraph->bidiParagraph = NULL;

    ListInitialize(&paragraph->scripts, sizeof(SBScript));
}

static void FinalizeTextParagraph(TextParagraphRef paragraph)
{
    SBParagraphRef bidiParagraph = paragraph->bidiParagraph;

    if (bidiParagraph) {
        SBParagraphRelease(bidiParagraph);
    }

    ListFinalize(&paragraph->scripts);
}

/**
 * Comparison function for binary search to locate a paragraph containing a specific code unit.
 */
static int ParagraphIndexComparison(const void *key, const void *element)
{
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

SB_INTERNAL SBUInteger TextAnalysisGetCodeUnitParagraphIndex(TextAnalysisRef analysis,
    SBUInteger codeUnitIndex)
{
    TextParagraph *array = analysis->paragraphs.items;
    SBUInteger count = analysis->paragraphs.count;
    void *item = NULL;

    if (array) {
        item = bsearch(&codeUnitIndex, array, count, sizeof(TextParagraph), ParagraphIndexComparison);
    }

    if (item) {
        return (SBUInteger)((TextParagraph *)item - array);
    }

    return SBInvalidIndex;
}

SB_INTERNAL void TextAnalysisGetBoundaryParagraphs(TextAnalysisRef analysis, SBUInteger codeUnitCount,
    SBUInteger rangeStart, SBUInteger rangeEnd,
    TextParagraphRef *firstParagraph, TextParagraphRef *lastParagraph)
{
    SBAssert(firstParagraph && lastParagraph);

    *firstParagraph = NULL;
    *lastParagraph = NULL;

    /* Find the first paragraph intersecting the range */
    if (rangeStart < codeUnitCount) {
        SBUInteger paragraphIndex;
        SBUInteger paragraphStart;
        SBUInteger paragraphEnd;

        paragraphIndex = TextAnalysisGetCodeUnitParagraphIndex(analysis, rangeStart);
        *firstParagraph = ListGetRef(&analysis->paragraphs, paragraphIndex);

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

        paragraphIndex = TextAnalysisGetCodeUnitParagraphIndex(analysis, rangeEnd - 1);
        *lastParagraph = ListGetRef(&analysis->paragraphs, paragraphIndex);
    }
}

/* =========================================================================
 * Paragraph Segmentation
 * ========================================================================= */

static TextParagraphRef InsertEmptyParagraph(TextAnalysisRef analysis, SBUInteger listIndex)
{
    SBBoolean succeeded;
    TextParagraph paragraph;

    InitializeTextParagraph(&paragraph);
    succeeded = ListInsert(&analysis->paragraphs, listIndex, &paragraph);

    return (succeeded ? ListGetRef(&analysis->paragraphs, listIndex) : NULL);
}

static void RemoveParagraphRange(TextAnalysisRef analysis, SBUInteger index, SBUInteger length)
{
    SBUInteger endIndex = index + length;
    SBUInteger paragraphIndex;

    /* Finalize each paragraph's resources */
    for (paragraphIndex = index; paragraphIndex < endIndex; paragraphIndex++) {
        TextParagraphRef paragraph = ListGetRef(&analysis->paragraphs, paragraphIndex);
        FinalizeTextParagraph(paragraph);
    }

    ListRemoveRange(&analysis->paragraphs, index, length);
}

/**
 * Adjusts the start index of all paragraphs from a given position onward by a delta.
 */
static void ShiftParagraphRanges(TextAnalysisRef analysis, SBUInteger listIndex, SBInteger indexDelta)
{
    while (listIndex < analysis->paragraphs.count) {
        TextParagraphRef paragraph = ListGetRef(&analysis->paragraphs, listIndex);
        paragraph->index += indexDelta;
        listIndex += 1;
    }
}

/* =========================================================================
 * Replacement / Flush
 * ========================================================================= */

static void UpdateParagraphsForTextReplacement(TextAnalysisRef analysis, TextBufferRef buffer,
    BidiTypesBufferRef bidiTypesBuffer, SBUInteger replaceStart, SBUInteger oldLength, SBUInteger newLength)
{
    SBUInteger oldEnd = replaceStart + oldLength;
    SBUInteger newEnd = replaceStart + newLength;
    SBInteger lengthDelta = (SBInteger)(newLength - oldLength);
    SBUInteger paragraphIndex;
    SBUInteger removalEnd;
    SBCodepointSequence sequence;
    TextParagraphRef paragraph;
    SBUInteger scanIndex;

    /* Find the first affected paragraph */
    paragraphIndex = TextAnalysisGetCodeUnitParagraphIndex(analysis, replaceStart > 0 ? replaceStart - 1 : 0);
    if (paragraphIndex == SBInvalidIndex) {
        paragraphIndex = analysis->paragraphs.count;
    }

    /* Determine starting point for scanning */
    if (paragraphIndex < analysis->paragraphs.count) {
        paragraph = ListGetRef(&analysis->paragraphs, paragraphIndex);
        scanIndex = paragraph->index;
    } else {
        scanIndex = replaceStart;
    }

    /* Setup for scanning */
    TextBufferGetCodepointSequence(buffer, &sequence);

    while (scanIndex < sequence.stringLength) {
        SBUInteger separatorLength;
        SBUInteger paraLength;

        SBCodepointSequenceGetParagraphBoundary(&sequence, BidiTypesBufferGetPtr(bidiTypesBuffer, 0),
            scanIndex, sequence.stringLength - scanIndex, &paraLength, &separatorLength);

        /* Get or create paragraph slot */
        if (paragraphIndex < analysis->paragraphs.count) {
            paragraph = ListGetRef(&analysis->paragraphs, paragraphIndex);

            /* Check if this slot is within reusable range */
            if (paragraph->index > oldEnd) {
                /* Slot is after affected region, insert new one */
                paragraph = InsertEmptyParagraph(analysis, paragraphIndex);
            } else {
                SBUInteger paragraphEnd = paragraph->index + paragraph->length;

                /* Check for splitting */
                if (paragraphEnd > oldEnd && separatorLength > 0) {
                    newEnd = paragraphEnd + lengthDelta;
                }
            }
        } else {
            /* Need new slot */
            paragraph = InsertEmptyParagraph(analysis, paragraphIndex);
        }

        /* Update paragraph */
        paragraph->index = scanIndex;
        paragraph->length = paraLength;
        paragraph->needsReanalysis = SBTrue;

        scanIndex += paraLength;
        paragraphIndex += 1;

        if (scanIndex > replaceStart && scanIndex >= newEnd) {
            break;
        }
    }

    /* Remove any leftover slots that weren't reused */
    removalEnd = paragraphIndex;
    while (removalEnd < analysis->paragraphs.count) {
        paragraph = ListGetRef(&analysis->paragraphs, removalEnd);
        if (paragraph->index > oldEnd) {
            break;
        }

        removalEnd += 1;
    }

    RemoveParagraphRange(analysis, paragraphIndex, removalEnd - paragraphIndex);

    /* Shift paragraphs after the affected region */
    if (lengthDelta != 0 && paragraphIndex < analysis->paragraphs.count) {
        ShiftParagraphRanges(analysis, paragraphIndex, lengthDelta);
    }
}

SB_INTERNAL void TextAnalysisReplaceRange(TextAnalysisRef analysis, TextBufferRef buffer,
    BidiTypesBufferRef bidiTypesBuffer, SBUInteger replaceStart, SBUInteger oldLength, SBUInteger newLength)
{
    UpdateParagraphsForTextReplacement(analysis, buffer, bidiTypesBuffer, replaceStart, oldLength, newLength);
}

static void GenerateBidiParagraph(TextAnalysisRef analysis, TextBufferRef buffer,
    BidiTypesBufferRef bidiTypesBuffer, TextParagraphRef paragraph)
{
    SBCodepointSequence codepointSequence;
    const SBBidiType *bidiTypes;

    TextBufferGetCodepointSequence(buffer, &codepointSequence);
    bidiTypes = BidiTypesBufferGetPtr(bidiTypesBuffer, 0);

    if (paragraph->bidiParagraph) {
        /* Release old bidi paragraph */
        SBParagraphRelease(paragraph->bidiParagraph);
        paragraph->bidiParagraph = NULL;
    }

    paragraph->bidiParagraph = SBParagraphCreateWithCodepointSequence(
        &codepointSequence, bidiTypes, paragraph->index, paragraph->length, analysis->baseLevel);
}

static void PopulateParagraphScripts(TextAnalysisRef analysis, TextBufferRef buffer,
    TextParagraphRef paragraph)
{
    SBScriptLocatorRef scriptLocator;
    SBCodepointSequence codepointSequence;
    const SBScriptAgent *scriptAgent;

    scriptLocator = analysis->scriptLocator;

    codepointSequence.stringEncoding = buffer->encoding;
    codepointSequence.stringBuffer = TextBufferGetCodeUnitsPtr(buffer, paragraph->index);
    codepointSequence.stringLength = paragraph->length;

    ListRemoveAll(&paragraph->scripts);
    ListReserveRange(&paragraph->scripts, 0, paragraph->length);

    scriptAgent = &scriptLocator->agent;
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

SB_INTERNAL void TextAnalysisFlush(TextAnalysisRef analysis, TextBufferRef buffer,
    BidiTypesBufferRef bidiTypesBuffer)
{
    SBUInteger paragraphCount = analysis->paragraphs.count;
    SBUInteger paragraphIndex;

    for (paragraphIndex = 0; paragraphIndex < paragraphCount; paragraphIndex++) {
        TextParagraphRef paragraph = ListGetRef(&analysis->paragraphs, paragraphIndex);

        if (paragraph->needsReanalysis) {
            GenerateBidiParagraph(analysis, buffer, bidiTypesBuffer, paragraph);
            PopulateParagraphScripts(analysis, buffer, paragraph);

            paragraph->needsReanalysis = SBFalse;
        }
    }
}

/* =========================================================================
 * Queries
 * ========================================================================= */

static void CopyPerParagraphArray(TextAnalysisRef analysis, SBUInteger index, SBUInteger length,
    CopyArrayKind kind, void *buffer)
{
    SBUInteger rangeStart = index;
    SBUInteger rangeEnd = index + length;
    SBUInteger paragraphIndex = TextAnalysisGetCodeUnitParagraphIndex(analysis, rangeStart);
    SBUInt8 *cursor = buffer;

    while (rangeStart < rangeEnd) {
        const TextParagraph *textParagraph = ListGetRef(&analysis->paragraphs, paragraphIndex);
        SBUInteger copyStart = textParagraph->index;
        SBUInteger copyEnd = copyStart + textParagraph->length;
        SBUInteger copyCount;
        SBUInteger byteCount;
        const void *source;

        /* Clamp copy range to requested range */
        if (copyStart < rangeStart) {
            copyStart = rangeStart;
        }
        if (copyEnd > rangeEnd) {
            copyEnd = rangeEnd;
        }

        copyCount = copyEnd - copyStart;

        if (kind == CopyArrayScripts) {
            source = ListGetRef(&textParagraph->scripts, copyStart - textParagraph->index);
            byteCount = copyCount * sizeof(SBScript);
        } else {
            SBParagraphRef bidiParagraph = textParagraph->bidiParagraph;

            source = &bidiParagraph->fixedLevels[copyStart - textParagraph->index];
            byteCount = copyCount * sizeof(SBLevel);
        }

        memcpy(cursor, source, byteCount);

        cursor += byteCount;
        rangeStart = copyEnd;
        paragraphIndex += 1;
    }
}

SB_INTERNAL void TextAnalysisGetScripts(TextAnalysisRef analysis, SBUInteger index, SBUInteger length,
    SBScript *buffer)
{
    CopyPerParagraphArray(analysis, index, length, CopyArrayScripts, buffer);
}

SB_INTERNAL void TextAnalysisGetResolvedLevels(TextAnalysisRef analysis, SBUInteger index,
    SBUInteger length, SBLevel *buffer)
{
    CopyPerParagraphArray(analysis, index, length, CopyArrayLevels, buffer);
}

SB_INTERNAL void TextAnalysisGetCodeUnitParagraphInfo(TextAnalysisRef analysis, SBUInteger index,
    SBParagraphInfo *paragraphInfo)
{
    SBUInteger paragraphIndex = TextAnalysisGetCodeUnitParagraphIndex(analysis, index);
    const TextParagraph *textParagraph = ListGetRef(&analysis->paragraphs, paragraphIndex);
    SBParagraphRef bidiParagraph = textParagraph->bidiParagraph;

    paragraphInfo->index = textParagraph->index;
    paragraphInfo->length = textParagraph->length;
    paragraphInfo->baseLevel = bidiParagraph->baseLevel;
}

/* =========================================================================
 * Lifecycle
 * ========================================================================= */

SB_INTERNAL void TextAnalysisInitialize(TextAnalysisRef analysis, SBLevel baseLevel)
{
    analysis->baseLevel = baseLevel;
    analysis->scriptLocator = SBScriptLocatorCreate();

    ListInitialize(&analysis->paragraphs, sizeof(TextParagraph));
}

SB_INTERNAL void TextAnalysisCopyParagraphs(TextAnalysisRef analysis, const TextAnalysis *source)
{
    SBUInteger paragraphCount;
    SBUInteger paragraphIndex;

    /* Clear any existing paragraphs, releasing their owned resources first */
    RemoveParagraphRange(analysis, 0, analysis->paragraphs.count);

    paragraphCount = source->paragraphs.count;
    ListReserveRange(&analysis->paragraphs, 0, paragraphCount);

    for (paragraphIndex = 0; paragraphIndex < paragraphCount; paragraphIndex++) {
        const TextParagraph *sourceParagraph = ListGetRef(&source->paragraphs, paragraphIndex);
        TextParagraphRef destParagraph = ListGetRef(&analysis->paragraphs, paragraphIndex);

        destParagraph->index = sourceParagraph->index;
        destParagraph->length = sourceParagraph->length;
        ListInitialize(&destParagraph->scripts, sizeof(SBScript));

        if (sourceParagraph->needsReanalysis) {
            destParagraph->needsReanalysis = SBTrue;
            destParagraph->bidiParagraph = NULL;
        } else {
            SBUInteger scriptCount = sourceParagraph->scripts.count;
            SBUInteger byteCount;

            destParagraph->needsReanalysis = SBFalse;
            destParagraph->bidiParagraph = SBParagraphRetain(sourceParagraph->bidiParagraph);

            ListReserveRange(&destParagraph->scripts, 0, scriptCount);
            byteCount = scriptCount * sizeof(SBScript);
            memcpy(destParagraph->scripts.items, sourceParagraph->scripts.items, byteCount);
        }
    }
}

SB_INTERNAL void TextAnalysisFinalize(TextAnalysisRef analysis)
{
    SBUInteger paragraphIndex;

    for (paragraphIndex = 0; paragraphIndex < analysis->paragraphs.count; paragraphIndex++) {
        FinalizeTextParagraph(ListGetRef(&analysis->paragraphs, paragraphIndex));
    }

    ListFinalize(&analysis->paragraphs);

    if (analysis->scriptLocator) {
        SBScriptLocatorRelease(analysis->scriptLocator);
    }
}

#endif
