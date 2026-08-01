/*
 * Copyright (C) 2025-2026 Muhammad Tayyab Akram
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

#include <API/SBAssert.h>
#include <API/SBAttributeList.h>
#include <API/SBAttributeRegistry.h>
#include <API/SBLine.h>
#include <API/SBParagraph.h>
#include <API/SBText.h>
#include <Core/List.h>
#include <Core/Object.h>
#include <Text/AttributeDictionary.h>
#include <Text/AttributeManager.h>
#include <Text/BidiTypesBuffer.h>

#include "SBTextIterators.h"

/* ==========================================================================
 * Text Iterator Implementation
 * ========================================================================== */

/**
 * Resets a text iterator to process a specific range of text.
 *
 * @param iterator
 *      The text iterator to reset.
 * @param index
 *      The starting index of the range to process.
 * @param length
 *      The length of the range to process.
 */
static void ResetTextIterator(TextIteratorRef iterator, SBUInteger index, SBUInteger length);

/**
 * Initializes a text iterator with the specified text and direction mode.
 *
 * Prepares the iterator to traverse through the text. The direction mode determines whether the
 * iterator will follow visual ordering (for display) or logical ordering (for processing).
 *
 * @param iterator
 *      The text iterator to initialize.
 * @param text
 *      The text object to iterate over.
 * @param visualDirectionMode
 *      `SBTrue` for visual direction mode, `SBFalse` for logical direction mode.
 */
static void InitializeTextIterator(TextIteratorRef iterator, SBTextRef text,
    SBBoolean visualDirectionMode)
{
    iterator->text = SBTextRetain(text);
    iterator->visualDirectionMode = visualDirectionMode;

    ResetTextIterator(iterator, 0, text->buffer.codeUnits.count);
}

/**
 * Releases the text object held by the iterator and performs any necessary cleanup.
 *
 * @param iterator
 *      The text iterator to finalize.
 */
static void FinalizeTextIterator(TextIteratorRef iterator)
{
    SBTextRef text = iterator->text;

    if (text) {
        SBTextRelease(text);
    }
}

/**
 * Resets a text iterator to process a specific range of text.
 *
 * This function normalizes the input range and sets up the iterator state to begin processing text
 * from the specified position. If in visual direction mode, it determines the appropriate paragraph
 * and direction based on the text's bidirectional properties.
 *
 * @param iterator
 *      The text iterator to reset.
 * @param index
 *      The starting index of the range to process.
 * @param length
 *      The length of the range to process.
 */
static void ResetTextIterator(TextIteratorRef iterator, SBUInteger index, SBUInteger length)
{
    SBTextRef text = iterator->text;
    SBUInteger startIndex;
    SBUInteger endIndex;
    SBBoolean forwardMode;
    SBUInteger paragraphIndex;

    SBUIntegerNormalizeRange(text->buffer.codeUnits.count, &index, &length);

    /* Setup iterator boundary */
    startIndex = index;
    endIndex = index + length;
    forwardMode = SBTrue;
    paragraphIndex = SBInvalidIndex;

    if (length > 0) {
        /* Find out the index of the first paragraph */
        paragraphIndex = TextAnalysisGetCodeUnitParagraphIndex((TextAnalysisRef)&text->analysis, index);

        if (iterator->visualDirectionMode) {
            TextParagraphRef textParagraph = ListGetRef(&text->analysis.paragraphs, paragraphIndex);
            SBParagraphRef bidiParagraph = textParagraph->bidiParagraph;

            forwardMode = (bidiParagraph->baseLevel & 1) == 0;

            if (!forwardMode) {
                SBUInteger paragraphEnd = textParagraph->index + textParagraph->length;

                if (paragraphEnd < endIndex) {
                    paragraphIndex = TextAnalysisGetCodeUnitParagraphIndex((TextAnalysisRef)&text->analysis, endIndex - 1);
                }
            }
        }
    }

    /* Initialize the iterator state */
    iterator->forwardMode = forwardMode;
    iterator->startIndex = startIndex;
    iterator->endIndex = endIndex;
    iterator->paragraphIndex = paragraphIndex;

    /* Initialize the current element info to NULL */
    iterator->currentParagraph = NULL;
    iterator->paragraphStart = SBInvalidIndex;
    iterator->paragraphEnd = SBInvalidIndex;
}

static SBBoolean AdvanceTextIterator(TextIteratorRef iterator)
{
    /* Get the text reference and calculate the remaining length */
    SBTextRef text = iterator->text;
    SBUInteger remainingLength = iterator->endIndex - iterator->startIndex;

    if (remainingLength > 0) {
        /* Get the current paragraph and its boundaries */
        TextParagraphRef textParagraph = ListGetRef(&text->analysis.paragraphs, iterator->paragraphIndex);
        SBUInteger paragraphStart = textParagraph->index;
        SBUInteger paragraphEnd = paragraphStart + textParagraph->length;

        /* Adjust the paragraph boundaries to iterator range */
        if (paragraphStart <= iterator->startIndex) {
            paragraphStart = iterator->startIndex;
        }
        if (paragraphEnd >= iterator->endIndex) {
            paragraphEnd = iterator->endIndex;
        }

        /* Initialize the current element info */
        iterator->currentParagraph = textParagraph;
        iterator->paragraphStart = paragraphStart;
        iterator->paragraphEnd = paragraphEnd;

        /* Update iterator position based on the direction */
        if (iterator->forwardMode) {
            /* Move forward to the next paragraph */
            iterator->startIndex = paragraphEnd;
            iterator->paragraphIndex += 1;
        } else {
            /* Move backward to the previous paragraph */
            iterator->endIndex = paragraphStart;
            iterator->paragraphIndex -= 1;
        }

        return SBTrue;
    }

    /* No more text to process */
    return SBFalse;
}

/* ==========================================================================
 * Paragraph Iterator Implementation
 * ========================================================================== */

/**
 * Initializes a paragraph information structure.
 *
 * Sets default values for a paragraph's properties including its position, length, and base
 * embedding level.
 *
 * @param info
 *      Pointer to the paragraph info structure to initialize.
 */
static void InitializeParagraphInfo(SBParagraphInfo *info)
{
    info->index = SBInvalidIndex;
    info->length = 0;
    info->baseLevel = 0;
    info->userInfo = NULL;
}

/**
 * Cleans up resources associated with a paragraph iterator, including its parent text iterator.
 *
 * @param object
 *      The paragraph iterator to finalize.
 */
static void FinalizeParagraphIterator(ObjectRef object)
{
    SBParagraphIteratorRef iterator = object;
    FinalizeTextIterator(&iterator->parent);
}

SB_INTERNAL SBParagraphIteratorRef SBParagraphIteratorCreate(SBTextRef text)
{
    const SBUInteger size = sizeof(SBParagraphIterator);
    void *pointer = NULL;
    SBParagraphIteratorRef iterator;

    /* Text MUST be available. */
    SBAssert(text != NULL);

    iterator = ObjectCreate(&size, 1, &pointer, FinalizeParagraphIterator);

    if (iterator) {
        InitializeTextIterator(&iterator->parent, text, SBFalse);
        InitializeParagraphInfo(&iterator->currentParagraph);
    }

    return iterator;
}

SBTextRef SBParagraphIteratorGetText(SBParagraphIteratorRef iterator)
{
    return iterator->parent.text;
}

void SBParagraphIteratorReset(SBParagraphIteratorRef iterator, SBUInteger index, SBUInteger length)
{
    ResetTextIterator(&iterator->parent, index, length);
    InitializeParagraphInfo(&iterator->currentParagraph);
}

const SBParagraphInfo *SBParagraphIteratorGetCurrent(SBParagraphIteratorRef iterator)
{
    return &iterator->currentParagraph;
}

SBBoolean SBParagraphIteratorMoveNext(SBParagraphIteratorRef iterator)
{
    TextIteratorRef parent = &iterator->parent;

    if (AdvanceTextIterator(parent)) {
        TextParagraphRef textParagraph = parent->currentParagraph;
        SBParagraphInfo *currentInfo;

        currentInfo = &iterator->currentParagraph;
        currentInfo->index = parent->paragraphStart;
        currentInfo->length = parent->paragraphEnd - parent->paragraphStart;
        currentInfo->baseLevel = textParagraph->bidiParagraph->baseLevel;
        currentInfo->userInfo = textParagraph->userInfo;

        return SBTrue;
    }

    return SBFalse;
}

SBParagraphIteratorRef SBParagraphIteratorRetain(SBParagraphIteratorRef iterator)
{
    return ObjectRetain(iterator);
}

void SBParagraphIteratorRelease(SBParagraphIteratorRef iterator)
{
    ObjectRelease(iterator);
}

/* ==========================================================================
 * Logical Run Iterator Implementation
 * ========================================================================== */

/**
 * Initializes a logical run structure.
 *
 * Sets default values for a logical run's properties including its position, length, and
 * bidirectional embedding level.
 *
 * @param run
 *      Pointer to the logical run structure to initialize.
 */
static void InitializeLogicalRun(SBLogicalRun *run)
{
    run->index = SBInvalidIndex;
    run->length = 0;
    run->level = 0;
}

/**
 * Cleans up resources associated with a logical run iterator, including its parent text iterator.
 *
 * @param object
 *      The logical run iterator to finalize.
 */
static void FinalizeLogicalRunIterator(ObjectRef object)
{
    SBLogicalRunIteratorRef iterator = object;
    FinalizeTextIterator(&iterator->parent);
}

SB_INTERNAL SBLogicalRunIteratorRef SBLogicalRunIteratorCreate(SBTextRef text)
{
    const SBUInteger size = sizeof(SBLogicalRunIterator);
    void *pointer = NULL;
    SBLogicalRunIteratorRef iterator;

    /* Text MUST be available. */
    SBAssert(text != NULL);

    iterator = ObjectCreate(&size, 1, &pointer, FinalizeLogicalRunIterator);

    if (iterator) {
        InitializeTextIterator(&iterator->parent, text, SBFalse);
        InitializeLogicalRun(&iterator->currentRun);

        iterator->levelIndex = SBInvalidIndex;
    }

    return iterator;
}

SBTextRef SBLogicalRunIteratorGetText(SBLogicalRunIteratorRef iterator)
{
    return iterator->parent.text;
}

void SBLogicalRunIteratorReset(SBLogicalRunIteratorRef iterator, SBUInteger index, SBUInteger length)
{
    ResetTextIterator(&iterator->parent, index, length);
    InitializeLogicalRun(&iterator->currentRun);

    iterator->levelIndex = SBInvalidIndex;
}

const SBLogicalRun *SBLogicalRunIteratorGetCurrent(SBLogicalRunIteratorRef iterator)
{
    return &iterator->currentRun;
}

SBBoolean SBLogicalRunIteratorMoveNext(SBLogicalRunIteratorRef iterator)
{
    /* Get parent iterator and current paragraph */
    TextIteratorRef parent = &iterator->parent;
    TextParagraphRef textParagraph = parent->currentParagraph;

    /* Check if we need to load a new paragraph */
    if (iterator->levelIndex == SBInvalidIndex) {
        SBLogicalRun *currentRun = &iterator->currentRun;
        SBUInteger runStart = parent->startIndex;

        /* Attempt to load the next paragraph */
        if (AdvanceTextIterator(parent)) {
            textParagraph = parent->currentParagraph;
            iterator->levelIndex = 0;
            currentRun->index = runStart;
            currentRun->length = 0;
        } else {
            /* No more paragraphs available */
            textParagraph = NULL;
            InitializeLogicalRun(currentRun);
        }
    }

    if (textParagraph) {
        SBLogicalRun *currentRun = &iterator->currentRun;
        SBUInteger paragraphLength = parent->paragraphEnd - parent->paragraphStart;
        SBUInteger currentLevelStart = iterator->levelIndex;
        SBParagraphRef bidiParagraph;
        const SBLevel *embeddingLevels;
        SBLevel currentLevel;

        /* Get bidirectional information for the paragraph */
        bidiParagraph = textParagraph->bidiParagraph;
        embeddingLevels = &bidiParagraph->fixedLevels[parent->paragraphStart - textParagraph->index];
        currentLevel = embeddingLevels[iterator->levelIndex];

        /* Find the end of the current level run */
        while (++iterator->levelIndex < paragraphLength) {
            if (embeddingLevels[iterator->levelIndex] != currentLevel) {
                break;
            }
        }

        /* Update the run information */
        currentRun->index += currentRun->length;
        currentRun->length = iterator->levelIndex - currentLevelStart;
        currentRun->level = currentLevel;

        /* Check if the end of the paragraph is reached */
        if (iterator->levelIndex == paragraphLength) {
            /* Prepare for the next paragraph */
            iterator->levelIndex = SBInvalidIndex;
        }

        return SBTrue;
    }

    /* No more runs available */
    return SBFalse;
}

SBLogicalRunIteratorRef SBLogicalRunIteratorRetain(SBLogicalRunIteratorRef iterator)
{
    return ObjectRetain(iterator);
}

void SBLogicalRunIteratorRelease(SBLogicalRunIteratorRef iterator)
{
    ObjectRelease(iterator);
}

/* ==========================================================================
 * Script Run Iterator Implementation
 * ========================================================================== */

/**
 * Initializes a script run structure.
 *
 * Sets default values for a script run's properties including its position, length, and writing
 * script identifier.
 *
 * @param run
 *      Pointer to the script run structure to initialize.
 */
static void InitializeScriptRun(SBScriptRun *run)
{
    run->index = SBInvalidIndex;
    run->length = 0;
    run->script = SBScriptNil;
}

/**
 * Cleans up resources associated with a script run iterator, including its parent text iterator.
 *
 * @param object The script run iterator to finalize.
 */
static void FinalizeScriptRunIterator(ObjectRef object)
{
    SBScriptRunIteratorRef iterator = object;
    FinalizeTextIterator(&iterator->parent);
}

SB_INTERNAL SBScriptRunIteratorRef SBScriptRunIteratorCreate(SBTextRef text)
{
    const SBUInteger size = sizeof(SBScriptRunIterator);
    void *pointer = NULL;
    SBScriptRunIteratorRef iterator;

    /* Text MUST be available. */
    SBAssert(text != NULL);

    iterator = ObjectCreate(&size, 1, &pointer, FinalizeScriptRunIterator);

    if (iterator) {
        InitializeTextIterator(&iterator->parent, text, SBFalse);
        InitializeScriptRun(&iterator->currentRun);

        iterator->scriptIndex = SBInvalidIndex;
    }

    return iterator;
}

SBTextRef SBScriptRunIteratorGetText(SBScriptRunIteratorRef iterator)
{
    return iterator->parent.text;
}

void SBScriptRunIteratorReset(SBScriptRunIteratorRef iterator, SBUInteger index, SBUInteger length)
{
    ResetTextIterator(&iterator->parent, index, length);
    InitializeScriptRun(&iterator->currentRun);

    iterator->scriptIndex = SBInvalidIndex;
}

const SBScriptRun *SBScriptRunIteratorGetCurrent(SBScriptRunIteratorRef iterator)
{
    return &iterator->currentRun;
}

SBBoolean SBScriptRunIteratorMoveNext(SBScriptRunIteratorRef iterator)
{
    /* Get parent iterator and current paragraph */
    TextIteratorRef parent = &iterator->parent;
    TextParagraphRef textParagraph = parent->currentParagraph;

    /* Check if there's a need to load a new paragraph */
    if (iterator->scriptIndex == SBInvalidIndex) {
        SBScriptRun *currentRun = &iterator->currentRun;
        SBUInteger runStart = parent->startIndex;

        /* Attempt to load the next paragraph */
        if (AdvanceTextIterator(parent)) {
            textParagraph = parent->currentParagraph;
            iterator->scriptIndex = 0;
            currentRun->index = runStart;
            currentRun->length = 0;
        } else {
            /* No more paragraphs available */
            textParagraph = NULL;
            InitializeScriptRun(currentRun);
        }
    }

    if (textParagraph) {
        SBScriptRun *currentRun = &iterator->currentRun;
        SBUInteger paragraphLength = parent->paragraphEnd - parent->paragraphStart;
        SBUInteger scriptStart = iterator->scriptIndex;
        const SBScript *scriptArray;
        SBScript currentScript;

        /* Get script information for the paragraph */
        scriptArray = &textParagraph->scripts.items[parent->paragraphStart - textParagraph->index];
        currentScript = scriptArray[iterator->scriptIndex];

        /* Find the end of the current script run */
        while (++iterator->scriptIndex < paragraphLength) {
            if (scriptArray[iterator->scriptIndex] != currentScript) {
                break;
            }
        }

        /* Update the run information */
        currentRun->index += currentRun->length;
        currentRun->length = iterator->scriptIndex - scriptStart;
        currentRun->script = currentScript;

        /* Check if the end of the paragraph is reached */
        if (iterator->scriptIndex == paragraphLength) {
            /* Prepare for the next paragraph */
            iterator->scriptIndex = SBInvalidIndex;
        }

        return SBTrue;
    }

    /* No more runs available */
    return SBFalse;
}

SBScriptRunIteratorRef SBScriptRunIteratorRetain(SBScriptRunIteratorRef iterator)
{
    return ObjectRetain(iterator);
}

void SBScriptRunIteratorRelease(SBScriptRunIteratorRef iterator)
{
    ObjectRelease(iterator);
}

/* ==========================================================================
 * Attribute Filter Implementation
 * ========================================================================== */

SBAttributeFilter SBAttributeFilterMakeID(SBAttributeID attributeID)
{
    SBAttributeFilter filter;
    filter.kind = SBAttributeFilterKindID;
    filter.value.attributeID = attributeID;
    return filter;
}

SBAttributeFilter SBAttributeFilterMakeCollection(SBAttributeGroup attributeGroup,
    SBAttributeScope attributeScope)
{
    SBAttributeFilter filter;
    filter.kind = SBAttributeFilterKindCollection;
    filter.value.collection.group = attributeGroup;
    filter.value.collection.scope = attributeScope;
    return filter;
}

SBAttributeFilter SBAttributeFilterMakeAny(void)
{
    SBAttributeFilter filter;
    filter.kind = SBAttributeFilterKindAny;
    return filter;
}

/**
 * Resolves an `SBAttributeFilter` into the group/scope parameters expected by the group/scope
 * based attribute-filtering routines, treating `SBAttributeFilterKindAny` (and any other
 * non-collection kind) as "no group restriction, any scope."
 *
 * @param filter
 *      The filter to resolve.
 * @param filterGroup
 *      Receives the attribute group to filter by.
 * @param filterScope
 *      Receives the attribute scope to filter by.
 */
SB_INTERNAL void GetCollectionFilterParams(SBAttributeFilter filter, SBAttributeGroup *filterGroup,
    SBAttributeScope *filterScope)
{
    if (filter.kind == SBAttributeFilterKindCollection) {
        *filterGroup = filter.value.collection.group;
        *filterScope = filter.value.collection.scope;
    } else {
        *filterGroup = SBAttributeGroupNone;
        *filterScope = AttributeScopeAny;
    }
}

/* ==========================================================================
 * Uniform Run Iterator Implementation
 * ========================================================================== */

/**
 * Initializes a uniform run structure.
 *
 * Sets default values for a uniform run's properties including its position, length, level, script,
 * and attribute list.
 *
 * @param run
 *      Pointer to the uniform run structure to initialize.
 */
static void InitializeUniformRun(SBUniformRun *run)
{
    run->index = SBInvalidIndex;
    run->length = 0;
    run->level = 0;
    run->script = SBScriptNil;
    run->attributes = NULL;
}

/**
 * Cleans up resources associated with a uniform run iterator, including its attribute item list and
 * parent text iterator.
 *
 * @param object
 *      The uniform run iterator to finalize.
 */
static void FinalizeUniformRunIterator(ObjectRef object)
{
    SBUniformRunIteratorRef iterator = object;

    AttributeDictionaryFinalize(&iterator->items);
    FinalizeTextIterator(&iterator->parent);
}

SB_INTERNAL SBUniformRunIteratorRef SBUniformRunIteratorCreate(SBTextRef text)
{
    const SBUInteger size = sizeof(SBUniformRunIterator);
    void *pointer = NULL;
    SBUniformRunIteratorRef iterator;

    /* Text MUST be available. */
    SBAssert(text != NULL);

    iterator = ObjectCreate(&size, 1, &pointer, FinalizeUniformRunIterator);

    if (iterator) {
        InitializeTextIterator(&iterator->parent, text, SBFalse);
        InitializeUniformRun(&iterator->currentRun);

        iterator->rangeIndex = 0;
        iterator->rangeLength = text->buffer.codeUnits.count;
        iterator->boundaryIndex = SBInvalidIndex;
        iterator->filter = SBAttributeFilterMakeAny();

        AttributeDictionaryInitialize(&iterator->items, text->attributeRegistry);
    }

    return iterator;
}

SBTextRef SBUniformRunIteratorGetText(SBUniformRunIteratorRef iterator)
{
    return iterator->parent.text;
}

void SBUniformRunIteratorSetupFilter(SBUniformRunIteratorRef iterator, SBAttributeFilter filter)
{
    iterator->filter = filter;

    SBUniformRunIteratorReset(iterator, iterator->rangeIndex, iterator->rangeLength);
}

void SBUniformRunIteratorReset(SBUniformRunIteratorRef iterator, SBUInteger index, SBUInteger length)
{
    iterator->rangeIndex = index;
    iterator->rangeLength = length;

    ResetTextIterator(&iterator->parent, index, length);
    InitializeUniformRun(&iterator->currentRun);

    iterator->boundaryIndex = SBInvalidIndex;
}

const SBUniformRun *SBUniformRunIteratorGetCurrent(SBUniformRunIteratorRef iterator)
{
    return &iterator->currentRun;
}

SBBoolean SBUniformRunIteratorMoveNext(SBUniformRunIteratorRef iterator)
{
    /* Get parent iterator and current paragraph */
    TextIteratorRef parent = &iterator->parent;
    TextParagraphRef textParagraph = parent->currentParagraph;

    /* Check if we need to load a new paragraph */
    if (iterator->boundaryIndex == SBInvalidIndex) {
        SBUniformRun *currentRun = &iterator->currentRun;
        SBUInteger runStart = parent->startIndex;

        /* Attempt to load the next paragraph */
        if (AdvanceTextIterator(parent)) {
            textParagraph = parent->currentParagraph;
            iterator->boundaryIndex = 0;
            currentRun->index = runStart;
            currentRun->length = 0;
        } else {
            /* No more paragraphs available */
            textParagraph = NULL;
            InitializeUniformRun(currentRun);
        }
    }

    if (textParagraph) {
        SBTextRef text = parent->text;
        AttributeManagerRef manager = (AttributeManagerRef)&text->attributeManager;
        SBUniformRun *currentRun = &iterator->currentRun;
        SBUInteger paragraphOffset = parent->paragraphStart - textParagraph->index;
        SBUInteger paragraphLength = parent->paragraphEnd - parent->paragraphStart;
        const SBLevel *embeddingLevels;
        const SBScript *scriptArray;
        SBUInteger runStart = iterator->boundaryIndex;
        SBLevel currentLevel;
        SBScript currentScript;
        SBUInteger levelEnd = runStart;
        SBUInteger scriptEnd = runStart;
        SBUInteger attributeEnd = parent->paragraphStart + runStart;
        SBUInteger mergedEnd;
        SBAttributeGroup filterGroup;
        SBAttributeScope filterScope;

        /* Get bidirectional and script information for the paragraph */
        embeddingLevels = &textParagraph->bidiParagraph->fixedLevels[paragraphOffset];
        scriptArray = &textParagraph->scripts.items[paragraphOffset];
        currentLevel = embeddingLevels[runStart];
        currentScript = scriptArray[runStart];

        /* Find the end of the current level run */
        while (++levelEnd < paragraphLength && embeddingLevels[levelEnd] == currentLevel) {
        }

        /* Find the end of the current script run */
        while (++scriptEnd < paragraphLength && scriptArray[scriptEnd] == currentScript) {
        }

        /* Find the end of the current attribute run, clamped to the paragraph boundary */
        if (iterator->filter.kind == SBAttributeFilterKindID) {
            AttributeManagerGetOnwardRunByFilteringID(manager, &attributeEnd, parent->paragraphEnd,
                iterator->filter.value.attributeID, &iterator->items);
        } else {
            GetCollectionFilterParams(iterator->filter, &filterGroup, &filterScope);
            AttributeManagerGetOnwardRunByFilteringCollection(manager, &attributeEnd, parent->paragraphEnd,
                filterScope, filterGroup, &iterator->items);
        }

        /* Take the nearest of the level, script, and attribute boundaries */
        mergedEnd = levelEnd;
        if (scriptEnd < mergedEnd) {
            mergedEnd = scriptEnd;
        }
        if (attributeEnd - parent->paragraphStart < mergedEnd) {
            mergedEnd = attributeEnd - parent->paragraphStart;
        }

        /* Update the run information */
        currentRun->index += currentRun->length;
        currentRun->length = mergedEnd - runStart;
        currentRun->level = currentLevel;
        currentRun->script = currentScript;
        currentRun->attributes = iterator->items._list;

        iterator->boundaryIndex = mergedEnd;

        /* Check if the end of the paragraph is reached */
        if (mergedEnd == paragraphLength) {
            /* Prepare for the next paragraph */
            iterator->boundaryIndex = SBInvalidIndex;
        }

        return SBTrue;
    }

    /* No more runs available */
    return SBFalse;
}

SBUniformRunIteratorRef SBUniformRunIteratorRetain(SBUniformRunIteratorRef iterator)
{
    return ObjectRetain(iterator);
}

void SBUniformRunIteratorRelease(SBUniformRunIteratorRef iterator)
{
    ObjectRelease(iterator);
}

/* ==========================================================================
 * Visual Run Iterator Implementation
 * ========================================================================== */

/**
 * Initializes a visual run structure.
 *
 * Sets default values for a visual run's properties including its position, length, and
 * bidirectional embedding level in visual order.
 *
 * @param run
 *      Pointer to the visual run structure to initialize.
 */
static void InitializeVisualRun(SBVisualRun *run)
{
    run->index = SBInvalidIndex;
    run->length = 0;
    run->level = 0;
}

/**
 * Cleans up resources associated with a visual run iterator, including the bidirectional line
 * object and parent text iterator.
 *
 * @param object
 *      The visual run iterator to finalize.
 */
static void FinalizeVisualRunIterator(ObjectRef object)
{
    SBVisualRunIteratorRef iterator = object;

    if (iterator->bidiLine) {
        SBLineRelease(iterator->bidiLine);
    }

    FinalizeTextIterator(&iterator->parent);
}

SB_INTERNAL SBVisualRunIteratorRef SBVisualRunIteratorCreate(SBTextRef text)
{
    const SBUInteger size = sizeof(SBVisualRunIterator);
    void *pointer = NULL;
    SBVisualRunIteratorRef iterator;

    /* Text MUST be available. */
    SBAssert(text != NULL);

    iterator = ObjectCreate(&size, 1, &pointer, FinalizeVisualRunIterator);

    if (iterator) {
        InitializeTextIterator(&iterator->parent, text, SBTrue);
        InitializeVisualRun(&iterator->currentRun);

        iterator->bidiLine = NULL;
        iterator->runIndex = SBInvalidIndex;
    }

    return iterator;
}

SBTextRef SBVisualRunIteratorGetText(SBVisualRunIteratorRef iterator)
{
    return iterator->parent.text;
}

void SBVisualRunIteratorReset(SBVisualRunIteratorRef iterator, SBUInteger index, SBUInteger length)
{
    ResetTextIterator(&iterator->parent, index, length);
    InitializeVisualRun(&iterator->currentRun);

    iterator->bidiLine = NULL;
    iterator->runIndex = SBInvalidIndex;
}

const SBVisualRun *SBVisualRunIteratorGetCurrent(SBVisualRunIteratorRef iterator)
{
    return &iterator->currentRun;
}

SBBoolean SBVisualRunIteratorMoveNext(SBVisualRunIteratorRef iterator)
{
    SBLineRef bidiLine = iterator->bidiLine;

    if (!bidiLine) {
        TextIteratorRef parentIterator = &iterator->parent;

        /* Try to advance to the next paragraph */
        if (AdvanceTextIterator(parentIterator)) {
            /* Get paragraph information */
            TextParagraphRef textParagraph = parentIterator->currentParagraph;
            SBParagraphRef bidiParagraph = textParagraph->bidiParagraph;
            SBUInteger paragraphStart;
            SBUInteger paragraphLength;

            /* Calculate paragraph boundaries */
            paragraphStart = parentIterator->paragraphStart;
            paragraphLength = parentIterator->paragraphEnd - paragraphStart;

            /* Create a new bidirectional line from the paragraph, sourcing bidi types fresh from
               the text's current buffer rather than trusting the paragraph's own cached offset */
            bidiLine = SBLineCreate(
                BidiTypesBufferGetPtr((BidiTypesBufferRef)&parentIterator->text->bidiTypes, paragraphStart),
                &bidiParagraph->fixedLevels[paragraphStart - textParagraph->index],
                bidiParagraph->baseLevel, bidiParagraph->stringEncoding,
                paragraphStart, paragraphLength);

            /* Initialize line processing */
            iterator->bidiLine = bidiLine;
            iterator->runIndex = 0;
        }

        /* Reset the current run state */
        InitializeVisualRun(&iterator->currentRun);
    }

    if (bidiLine) {
        SBVisualRun *currentRun = &iterator->currentRun;
        SBRun *bidiRun;

        /* Get the current bidirectional run */
        bidiRun = &bidiLine->fixedRuns[iterator->runIndex];

        /* Update the current run with bidirectional properties */
        currentRun->index = bidiRun->offset;
        currentRun->length = bidiRun->length;
        currentRun->level = bidiRun->level;

        /* Move to the next run in the line */
        iterator->runIndex += 1;

        /* Check if all runs in the line are processed */
        if (iterator->runIndex == bidiLine->runCount) {
            /* Clean up and prepare for the next line */
            SBLineRelease(bidiLine);
            iterator->bidiLine = NULL;
        }

        return SBTrue;
    }

    /* No more runs available */
    return SBFalse;
}

SBVisualRunIteratorRef SBVisualRunIteratorRetain(SBVisualRunIteratorRef iterator)
{
    return ObjectRetain(iterator);
}

void SBVisualRunIteratorRelease(SBVisualRunIteratorRef iterator)
{
    ObjectRelease(iterator);
}

#endif
