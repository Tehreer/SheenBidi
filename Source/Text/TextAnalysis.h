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

#ifndef _SB_INTERNAL_TEXT_ANALYSIS_H
#define _SB_INTERNAL_TEXT_ANALYSIS_H

#include <API/SBBase.h>

#if SB_TEXT_API_SUPPORTED

#include <SheenBidi/SBParagraph.h>
#include <SheenBidi/SBScript.h>
#include <SheenBidi/SBScriptLocator.h>
#include <SheenBidi/SBTextIterators.h>

#include <Core/List.h>
#include <Text/BidiTypesBuffer.h>
#include <Text/TextBuffer.h>

typedef struct _TextParagraph {
    SBUInteger index;
    SBUInteger length;
    SBBoolean needsReanalysis;
    SBParagraphRef bidiParagraph;
    LIST(SBScript) scripts;
} TextParagraph, *TextParagraphRef;

/**
 * Derives and owns paragraph segmentation, per-paragraph bidirectional levels, and per-paragraph
 * scripts for a `TextBuffer`, given its already-derived `BidiTypesBuffer`.
 *
 * Mutations mark affected paragraphs as needing reanalysis instead of eagerly re-deriving bidi
 * levels/scripts; `TextAnalysisFlush()` later reprocesses all paragraphs still marked as such.
 */
typedef struct _TextAnalysis {
    SBLevel baseLevel;
    SBScriptLocatorRef scriptLocator;
    LIST(TextParagraph) paragraphs;
} TextAnalysis, *TextAnalysisRef;

/**
 * Initializes an empty text analysis with the given base paragraph level.
 *
 * @param analysis
 *      The text analysis to initialize.
 * @param baseLevel
 *      Base bidirectional level used for paragraphs derived by this analysis.
 */
SB_INTERNAL void TextAnalysisInitialize(TextAnalysisRef analysis, SBLevel baseLevel);

/**
 * Replaces an already-initialized text analysis's paragraphs with a deep copy of another
 * analysis's paragraphs. The analysis must already be initialized (via `TextAnalysisInitialize()`)
 * with the same base level as `source`.
 *
 * Paragraphs that were already up to date in the source are retained as-is (their cached bidi
 * paragraphs are shared with source via retain — this is safe since nothing reads position- or
 * buffer-identity-dependent state out of a cached `SBParagraph` after creation; callers always
 * supply fresh bidi-types/levels references at the point of use instead); paragraphs that were
 * pending reanalysis in the source remain pending so a subsequent `TextAnalysisFlush()`
 * regenerates them.
 *
 * @param analysis
 *      The text analysis to copy into.
 * @param source
 *      The source analysis to copy from.
 */
SB_INTERNAL void TextAnalysisCopyParagraphs(TextAnalysisRef analysis, const TextAnalysis *source);

/**
 * Finalizes a text analysis and releases all resources owned by its paragraphs.
 *
 * @param analysis
 *      The text analysis to finalize.
 */
SB_INTERNAL void TextAnalysisFinalize(TextAnalysisRef analysis);

/**
 * Finds the paragraph index containing the specified code unit index.
 *
 * @param analysis
 *      The text analysis to search.
 * @param codeUnitIndex
 *      The code unit index to search for.
 * @return
 *      The index of the paragraph containing the code unit, or `SBInvalidIndex` if not found.
 */
SB_INTERNAL SBUInteger TextAnalysisGetCodeUnitParagraphIndex(TextAnalysisRef analysis,
    SBUInteger codeUnitIndex);

/**
 * Retrieves the first and last paragraphs that intersect with a specified code unit range. If the
 * range spans a single paragraph, both output parameters reference the same paragraph.
 *
 * @param analysis
 *      The text analysis to search.
 * @param codeUnitCount
 *      Total number of code units currently held by the associated buffer.
 * @param rangeStart
 *      The starting code unit index (inclusive).
 * @param rangeEnd
 *      The ending code unit index (exclusive).
 * @param[out] firstParagraph
 *      Pointer to receive the first intersecting paragraph reference.
 * @param[out] lastParagraph
 *      Pointer to receive the last intersecting paragraph reference.
 */
SB_INTERNAL void TextAnalysisGetBoundaryParagraphs(TextAnalysisRef analysis, SBUInteger codeUnitCount,
    SBUInteger rangeStart, SBUInteger rangeEnd,
    TextParagraphRef *firstParagraph, TextParagraphRef *lastParagraph);

/**
 * Copies script identifications for a code-unit range.
 *
 * @param analysis
 *      The text analysis to read from.
 * @param index
 *      Start index (in code units).
 * @param length
 *      Number of code units to copy the scripts for.
 * @param buffer
 *      Output array of `SBScript` with `length` entries.
 */
SB_INTERNAL void TextAnalysisGetScripts(TextAnalysisRef analysis, SBUInteger index, SBUInteger length,
    SBScript *buffer);

/**
 * Copies resolved bidirectional levels for a code-unit range.
 *
 * @param analysis
 *      The text analysis to read from.
 * @param index
 *      Start index (in code units).
 * @param length
 *      Number of code units to copy the bidirectional levels for.
 * @param buffer
 *      Output array of `SBLevel` with `length` entries.
 */
SB_INTERNAL void TextAnalysisGetResolvedLevels(TextAnalysisRef analysis, SBUInteger index,
    SBUInteger length, SBLevel *buffer);

/**
 * Retrieves information for the paragraph containing a specific code unit.
 *
 * @param analysis
 *      The text analysis to read from.
 * @param index
 *      Code-unit index whose paragraph is queried.
 * @param paragraphInfo
 *      Output pointer to `SBParagraphInfo` to receive data.
 */
SB_INTERNAL void TextAnalysisGetCodeUnitParagraphInfo(TextAnalysisRef analysis, SBUInteger index,
    SBParagraphInfo *paragraphInfo);

/**
 * Notifies the analysis that a range of code units in `buffer` was replaced, re-deriving paragraph
 * boundaries immediately from the already-updated `bidiTypesBuffer`, and marking the affected
 * paragraphs as needing scripts and bidi-level (re)computation (see `TextAnalysisFlush()`).
 *
 * @param analysis
 *      The text analysis to update.
 * @param buffer
 *      The buffer the code units were replaced in (already updated by the caller).
 * @param bidiTypesBuffer
 *      The bidi-types buffer for `buffer` (already updated by the caller).
 * @param replaceStart
 *      Start index of the replaced range (in code units).
 * @param oldLength
 *      Length of the range before replacement.
 * @param newLength
 *      Length of the range after replacement.
 */
SB_INTERNAL void TextAnalysisReplaceRange(TextAnalysisRef analysis, TextBufferRef buffer,
    BidiTypesBufferRef bidiTypesBuffer, SBUInteger replaceStart, SBUInteger oldLength, SBUInteger newLength);

/**
 * Regenerates bidi levels and scripts for every paragraph still marked as needing reanalysis.
 *
 * @param analysis
 *      The text analysis to flush.
 * @param buffer
 *      The buffer to derive scripts from.
 * @param bidiTypesBuffer
 *      The bidi-types buffer to derive bidi levels from.
 */
SB_INTERNAL void TextAnalysisFlush(TextAnalysisRef analysis, TextBufferRef buffer,
    BidiTypesBufferRef bidiTypesBuffer);

#endif

#endif
