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
    paragraph->userInfo = NULL;

    ListInitialize(&paragraph->scripts, sizeof(SBScript));
}

static void FinalizeTextParagraph(TextAnalysisRef analysis, TextParagraphRef paragraph)
{
    SBParagraphRef bidiParagraph = paragraph->bidiParagraph;

    if (bidiParagraph) {
        SBParagraphRelease(bidiParagraph);
    }

    if (paragraph->userInfo) {
        if (analysis->userInfoCallbacks.release) {
            analysis->userInfoCallbacks.release(paragraph->userInfo);
        }
        paragraph->userInfo = NULL;
    }

    ListFinalize(&paragraph->scripts);
}

/**
 * Invokes the registered provider callback for a paragraph whose userInfo is NULL, storing
 * (and retaining) whatever it returns. No-op if the paragraph already has a userInfo, or if no
 * provider is registered.
 */
static void ProvideParagraphUserInfoIfNeeded(TextAnalysisRef analysis, TextParagraphRef paragraph)
{
    if (!paragraph->userInfo && analysis->userInfoProvider) {
        const void *provided = analysis->userInfoProvider(analysis->ownerText,
            paragraph->index, paragraph->length, analysis->userInfoProviderContext);

        if (provided) {
            if (analysis->userInfoCallbacks.retain) {
                provided = analysis->userInfoCallbacks.retain(provided);
            }
            paragraph->userInfo = provided;
        }
    }
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
        FinalizeTextParagraph(analysis, paragraph);
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

/*
 * Re-segments the paragraph list after a code-unit replacement.
 *
 * Strategy: identify the contiguous block of existing paragraphs whose boundaries the edit can
 * affect, discard exactly that block, re-scan the corresponding span of the (already updated) buffer
 * to regenerate fresh paragraphs, and shift the untouched survivors that follow.
 *
 * Correctness hinges on choosing the affected block precisely:
 *
 *   - First affected paragraph: the one containing `replaceStart - 1` (or the list head). Probing the
 *     code unit *before* the edit captures the case where the edit deletes a paragraph separator and
 *     merges the edited paragraph with its predecessor.
 *
 *   - First surviving paragraph: the first existing paragraph that starts strictly after `oldEnd`. Its
 *     leading separator sits at or beyond `oldEnd`, i.e. outside the replaced range `[replaceStart,
 *     oldEnd)`, so that separator (and hence the survivor's content and internal boundaries) is
 *     unaffected and only needs its start index shifted by `lengthDelta`. Because that separator still
 *     exists in the new buffer, the re-scan of the affected span naturally terminates exactly at the
 *     survivor's shifted start, keeping the two regions perfectly aligned.
 */
static void UpdateParagraphsForTextReplacement(TextAnalysisRef analysis, TextBufferRef buffer,
    BidiTypesBufferRef bidiTypesBuffer, SBUInteger replaceStart, SBUInteger oldLength, SBUInteger newLength)
{
    SBUInteger oldEnd = replaceStart + oldLength;
    SBInteger lengthDelta = (SBInteger)(newLength - oldLength);
    SBUInteger paragraphCount = analysis->paragraphs.count;
    SBUInteger firstIndex;
    SBUInteger survivorIndex;
    SBUInteger rescanStart;
    SBUInteger listIndex;
    SBUInteger scanIndex;
    SBCodepointSequence sequence;
    TextParagraphRef paragraph;

    /* Locate the first affected paragraph (see the note above for why replaceStart - 1 is probed). */
    firstIndex = TextAnalysisGetCodeUnitParagraphIndex(analysis, replaceStart > 0 ? replaceStart - 1 : 0);
    if (firstIndex == SBInvalidIndex) {
        firstIndex = paragraphCount;
    }

    if (firstIndex < paragraphCount) {
        paragraph = ListGetRef(&analysis->paragraphs, firstIndex);
        rescanStart = paragraph->index;
    } else {
        rescanStart = replaceStart;
    }

    /* Discard the paragraphs that start within the replaced range; they must be regenerated. Any
     * paragraph starting after the range is a potential survivor and is handled by the re-scan below. */
    for (survivorIndex = firstIndex; survivorIndex < paragraphCount; survivorIndex++) {
        paragraph = ListGetRef(&analysis->paragraphs, survivorIndex);
        if (paragraph->index > oldEnd) {
            break;
        }
    }
    RemoveParagraphRange(analysis, firstIndex, survivorIndex - firstIndex);

    TextBufferGetCodepointSequence(buffer, &sequence);

    /*
     * Re-scan the affected span, regenerating paragraphs. A surviving paragraph after the edited
     * region can be reused only when a freshly-computed boundary lands exactly on its (shifted) start;
     * this is always the case for well-formed text. If a regenerated paragraph instead overruns a
     * survivor's start - which can happen when an edit alters how later code units decode (e.g. a
     * malformed multi-byte sequence) so that a previous boundary no longer exists - that survivor is
     * absorbed and removed. Re-scanning therefore continues until it re-aligns with a survivor or
     * reaches the end of the text, guaranteeing a gap-free, overlap-free segmentation in all cases.
     */
    listIndex = firstIndex;
    scanIndex = rescanStart;

    while (scanIndex < sequence.stringLength) {
        SBUInteger separatorLength;
        SBUInteger paraLength;

        /* Re-align with, or absorb, the next survivor. */
        if (listIndex < analysis->paragraphs.count) {
            TextParagraphRef survivor = ListGetRef(&analysis->paragraphs, listIndex);
            SBUInteger survivorStart = (SBUInteger)(survivor->index + lengthDelta);

            if (scanIndex == survivorStart) {
                break;
            }
            if (survivorStart < scanIndex) {
                RemoveParagraphRange(analysis, listIndex, 1);
                continue;
            }
        }

        SBCodepointSequenceGetParagraphBoundary(&sequence, BidiTypesBufferGetPtr(bidiTypesBuffer, 0),
            scanIndex, sequence.stringLength - scanIndex, &paraLength, &separatorLength);

        paragraph = InsertEmptyParagraph(analysis, listIndex);
        paragraph->index = scanIndex;
        paragraph->length = paraLength;
        paragraph->needsReanalysis = SBTrue;

        scanIndex += paraLength;
        listIndex += 1;
    }

    /* Absorb any survivors overrun by the final regenerated paragraph (e.g. when the re-scan ran to
     * the end of the text without re-aligning on a survivor boundary). */
    while (listIndex < analysis->paragraphs.count) {
        TextParagraphRef survivor = ListGetRef(&analysis->paragraphs, listIndex);
        SBUInteger survivorStart = (SBUInteger)(survivor->index + lengthDelta);

        if (survivorStart >= scanIndex) {
            break;
        }
        RemoveParagraphRange(analysis, listIndex, 1);
    }

    /* Shift the untouched survivors that follow the regenerated region. */
    if (lengthDelta != 0) {
        ShiftParagraphRanges(analysis, listIndex, lengthDelta);
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

            ProvideParagraphUserInfoIfNeeded(analysis, paragraph);
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
    paragraphInfo->userInfo = textParagraph->userInfo;
}

SB_INTERNAL void TextAnalysisInvalidateParagraphUserInfo(TextAnalysisRef analysis, SBUInteger index,
    SBUInteger length)
{
    SBUInteger firstIndex;
    SBUInteger lastIndex;
    SBUInteger paragraphIndex;

    if (length == 0) {
        return;
    }

    firstIndex = TextAnalysisGetCodeUnitParagraphIndex(analysis, index);
    lastIndex = TextAnalysisGetCodeUnitParagraphIndex(analysis, index + length - 1);

    for (paragraphIndex = firstIndex; paragraphIndex <= lastIndex; paragraphIndex++) {
        TextParagraphRef paragraph = ListGetRef(&analysis->paragraphs, paragraphIndex);

        if (paragraph->userInfo) {
            if (analysis->userInfoCallbacks.release) {
                analysis->userInfoCallbacks.release(paragraph->userInfo);
            }
            paragraph->userInfo = NULL;
        }

        ProvideParagraphUserInfoIfNeeded(analysis, paragraph);
    }
}

/* =========================================================================
 * Lifecycle
 * ========================================================================= */

SB_INTERNAL void TextAnalysisInitialize(TextAnalysisRef analysis, SBTextRef ownerText,
    SBLevel baseLevel, const SBParagraphUserInfoCallbacks *userInfoCallbacks,
    SBParagraphUserInfoProviderCallback userInfoProvider, void *userInfoProviderContext)
{
    analysis->ownerText = ownerText;
    analysis->baseLevel = baseLevel;
    analysis->scriptLocator = SBScriptLocatorCreate();

    if (userInfoCallbacks) {
        analysis->userInfoCallbacks = *userInfoCallbacks;
    } else {
        analysis->userInfoCallbacks.retain = NULL;
        analysis->userInfoCallbacks.release = NULL;
    }

    analysis->userInfoProvider = userInfoProvider;
    analysis->userInfoProviderContext = userInfoProviderContext;

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
            destParagraph->userInfo = NULL;
        } else {
            SBUInteger scriptCount = sourceParagraph->scripts.count;
            SBUInteger byteCount;

            destParagraph->needsReanalysis = SBFalse;
            destParagraph->bidiParagraph = SBParagraphRetain(sourceParagraph->bidiParagraph);

            destParagraph->userInfo = sourceParagraph->userInfo;
            if (destParagraph->userInfo && analysis->userInfoCallbacks.retain) {
                destParagraph->userInfo = analysis->userInfoCallbacks.retain(destParagraph->userInfo);
            }

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
        FinalizeTextParagraph(analysis, ListGetRef(&analysis->paragraphs, paragraphIndex));
    }

    ListFinalize(&analysis->paragraphs);

    if (analysis->scriptLocator) {
        SBScriptLocatorRelease(analysis->scriptLocator);
    }
}

#endif
