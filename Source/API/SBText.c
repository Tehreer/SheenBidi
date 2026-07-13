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
#include <API/SBAttributeRegistry.h>
#include <API/SBTextConfig.h>
#include <API/SBTextIterators.h>
#include <Core/Object.h>
#include <Text/AttributeManager.h>
#include <Text/BidiTypesBuffer.h>
#include <Text/TextAnalysis.h>
#include <Text/TextBuffer.h>

#include "SBText.h"

/* =========================================================================
 * Text Implementation
 * ========================================================================= */

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
    return text->buffer.encoding;
}

SBAttributeRegistryRef SBTextGetAttributeRegistry(SBTextRef text)
{
    return text->attributeRegistry;
}

SBUInteger SBTextGetLength(SBTextRef text)
{
    return TextBufferGetLength((TextBufferRef)&text->buffer);
}

void SBTextGetCodeUnits(SBTextRef text, SBUInteger index, SBUInteger length, void *buffer)
{
    SBBoolean isRangeValid = SBUIntegerVerifyRange(text->buffer.codeUnits.count, index, length);

    SBAssert(isRangeValid);

    TextBufferGetCodeUnits((TextBufferRef)&text->buffer, index, length, buffer);
}

void SBTextGetBidiTypes(SBTextRef text, SBUInteger index, SBUInteger length, SBBidiType *buffer)
{
    SBBoolean isRangeValid = SBUIntegerVerifyRange(text->buffer.codeUnits.count, index, length);

    SBAssert(isRangeValid);

    BidiTypesBufferGetBidiTypes((BidiTypesBufferRef)&text->bidiTypes, index, length, buffer);
}

void SBTextGetScripts(SBTextRef text, SBUInteger index, SBUInteger length, SBScript *buffer)
{
    SBBoolean isRangeValid = SBUIntegerVerifyRange(text->buffer.codeUnits.count, index, length);

    SBAssert(isRangeValid && !text->isEditing);

    TextAnalysisGetScripts((TextAnalysisRef)&text->analysis, index, length, buffer);
}

void SBTextGetResolvedLevels(SBTextRef text, SBUInteger index, SBUInteger length, SBLevel *buffer)
{
    SBBoolean isRangeValid = SBUIntegerVerifyRange(text->buffer.codeUnits.count, index, length);

    SBAssert(isRangeValid && !text->isEditing);

    TextAnalysisGetResolvedLevels((TextAnalysisRef)&text->analysis, index, length, buffer);
}

void SBTextGetCodeUnitParagraphInfo(SBTextRef text, SBUInteger index,
    SBParagraphInfo *paragraphInfo)
{
    SBBoolean isValidIndex = index < text->buffer.codeUnits.count;

    SBAssert(isValidIndex && !text->isEditing);

    TextAnalysisGetCodeUnitParagraphInfo((TextAnalysisRef)&text->analysis, index, paragraphInfo);
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

SBUniformRunIteratorRef SBTextCreateUniformRunIterator(SBTextRef text)
{
    return SBUniformRunIteratorCreate(text);
}

SBVisualRunIteratorRef SBTextCreateVisualRunIterator(SBTextRef text,
    SBUInteger index, SBUInteger length)
{
    SBVisualRunIteratorRef iterator = SBVisualRunIteratorCreate(text);

    if (iterator) {
        SBVisualRunIteratorReset(iterator, index, length);
    }

    return iterator;
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

static void FinalizeMutableText(ObjectRef object)
{
    SBMutableTextRef text = object;

    AttributeManagerFinalize(&text->attributeManager);
    TextAnalysisFinalize(&text->analysis);
    BidiTypesBufferFinalize(&text->bidiTypes);
    TextBufferFinalize(&text->buffer);

    if (text->attributeRegistry) {
        SBAttributeRegistryRelease(text->attributeRegistry);
    }
}

SB_INTERNAL SBMutableTextRef SBTextCreateMutableWithParameters(SBStringEncoding encoding,
    SBAttributeRegistryRef attributeRegistry, SBLevel baseLevel,
    const SBParagraphUserInfoCallbacks *userInfoCallbacks,
    SBParagraphUserInfoProviderCallback userInfoProvider, void *userInfoProviderContext)
{
    const SBUInteger size = sizeof(SBText);
    void *pointer = NULL;
    SBMutableTextRef text;

    text = ObjectCreate(&size, 1, &pointer, FinalizeMutableText);

    if (text) {
        if (attributeRegistry) {
            attributeRegistry = SBAttributeRegistryRetain(attributeRegistry);
        }

        text->isMutable = SBTrue;
        text->isEditing = SBFalse;
        text->attributeRegistry = attributeRegistry;

        TextBufferInitialize(&text->buffer, encoding);
        BidiTypesBufferInitialize(&text->bidiTypes);
        TextAnalysisInitialize(&text->analysis, text, baseLevel, userInfoCallbacks,
            userInfoProvider, userInfoProviderContext);
        AttributeManagerInitialize(&text->attributeManager, &text->analysis, attributeRegistry);
    }

    return text;
}

SBMutableTextRef SBTextCreateMutable(SBStringEncoding encoding, SBTextConfigRef config)
{
    SBMutableTextRef text = SBTextCreateMutableWithParameters(encoding,
        config->attributeRegistry, config->baseLevel, &config->userInfoCallbacks,
        config->userInfoProvider, config->userInfoProviderContext);

    if (text) {
        /* TODO: Apply default attributes */
    }

    return text;
}

SBMutableTextRef SBTextCreateMutableCopy(SBTextRef text)
{
    SBMutableTextRef copy = SBTextCreateMutableWithParameters(text->buffer.encoding,
        text->attributeRegistry, text->analysis.baseLevel, &text->analysis.userInfoCallbacks,
        text->analysis.userInfoProvider, text->analysis.userInfoProviderContext);

    if (copy) {
        TextBufferCopyCodeUnits(&copy->buffer, &text->buffer);
        BidiTypesBufferCopyBidiTypes(&copy->bidiTypes, &text->bidiTypes);
        TextAnalysisCopyParagraphs(&copy->analysis, &text->analysis);
        TextAnalysisFlush(&copy->analysis, &copy->buffer, &copy->bidiTypes);

        /* Copy attributes */
        AttributeManagerCopyAttributes(&copy->attributeManager, &text->attributeManager);
    }

    return copy;
}

void SBTextBeginEditing(SBMutableTextRef text)
{
    SBAssert(text->isMutable);

    if (!text->isMutable) {
        return;
    }

    text->isEditing = SBTrue;
}

void SBTextEndEditing(SBMutableTextRef text)
{
    SBAssert(text->isMutable);

    if (!text->isMutable) {
        return;
    }

    TextAnalysisFlush(&text->analysis, &text->buffer, &text->bidiTypes);
    text->isEditing = SBFalse;
}

void SBTextAppendCodeUnits(SBMutableTextRef text,
    const void *codeUnitBuffer, SBUInteger codeUnitCount)
{
    SBAssert(text->isMutable);

    if (!text->isMutable) {
        return;
    }

    SBTextInsertCodeUnits(text, text->buffer.codeUnits.count, codeUnitBuffer, codeUnitCount);
}

void SBTextInsertCodeUnits(SBMutableTextRef text, SBUInteger index,
    const void *codeUnitBuffer, SBUInteger codeUnitCount)
{
    SBAssert(text->isMutable && index <= text->buffer.codeUnits.count);

    if (!text->isMutable) {
        return;
    }

    if (codeUnitCount > 0) {
        /* Splice into the raw code-unit storage */
        TextBufferInsertCodeUnits(&text->buffer, index, codeUnitBuffer, codeUnitCount);

        /* Re-derive bidi types immediately; paragraph boundaries need them as input */
        BidiTypesBufferReplaceRange(&text->bidiTypes, &text->buffer, index, 0, codeUnitCount);

        /* Re-derive paragraph boundaries; scripts/levels stay deferred */
        TextAnalysisReplaceRange(&text->analysis, &text->buffer, &text->bidiTypes, index, 0, codeUnitCount);

        /* Reserve attribute manager space */
        AttributeManagerReserveRange(&text->attributeManager, index, codeUnitCount);

        /* Perform immediate analysis if not in batch editing mode */
        if (!text->isEditing) {
            TextAnalysisFlush(&text->analysis, &text->buffer, &text->bidiTypes);
        }
    }
}

void SBTextDeleteCodeUnits(SBMutableTextRef text, SBUInteger index, SBUInteger length)
{
    SBUInteger rangeEnd = index + length;
    SBBoolean isRangeValid = (rangeEnd <= text->buffer.codeUnits.count && index <= rangeEnd);

    SBAssert(text->isMutable && isRangeValid);

    if (!text->isMutable) {
        return;
    }

    if (length > 0) {
        TextBufferDeleteCodeUnits(&text->buffer, index, length);
        BidiTypesBufferReplaceRange(&text->bidiTypes, &text->buffer, index, length, 0);
        TextAnalysisReplaceRange(&text->analysis, &text->buffer, &text->bidiTypes, index, length, 0);
        AttributeManagerRemoveRange(&text->attributeManager, index, length);

        if (!text->isEditing) {
            /* Perform immediate analysis if not in batch editing mode */
            TextAnalysisFlush(&text->analysis, &text->buffer, &text->bidiTypes);
        }
    }
}

void SBTextSetCodeUnits(SBMutableTextRef text,
    const void *codeUnitBuffer, SBUInteger codeUnitCount)
{
    SBAssert(text->isMutable);

    if (!text->isMutable) {
        return;
    }

    SBTextReplaceCodeUnits(text, 0, text->buffer.codeUnits.count, codeUnitBuffer, codeUnitCount);
}

void SBTextReplaceCodeUnits(SBMutableTextRef text, SBUInteger index, SBUInteger length,
    const void *codeUnitBuffer, SBUInteger codeUnitCount)
{
    SBUInteger rangeEnd = index + length;
    SBBoolean isRangeValid = (rangeEnd <= text->buffer.codeUnits.count && index <= rangeEnd);

    SBAssert(text->isMutable && isRangeValid);

    if (!text->isMutable) {
        return;
    }

    if (length > 0 || codeUnitCount > 0) {
        TextBufferReplaceCodeUnits(&text->buffer, index, length, codeUnitBuffer, codeUnitCount);
        BidiTypesBufferReplaceRange(&text->bidiTypes, &text->buffer, index, length, codeUnitCount);
        TextAnalysisReplaceRange(&text->analysis, &text->buffer, &text->bidiTypes, index, length, codeUnitCount);
        AttributeManagerReplaceRange(&text->attributeManager, index, length, codeUnitCount);

        if (!text->isEditing) {
            /* Perform immediate analysis if not in batch editing mode */
            TextAnalysisFlush(&text->analysis, &text->buffer, &text->bidiTypes);
        }
    }
}

void SBTextSetAttribute(SBMutableTextRef text, SBUInteger index, SBUInteger length,
    SBAttributeID attributeID, const void *attributeValue)
{
    SBUInteger rangeEnd = index + length;
    SBBoolean isRangeValid = (rangeEnd <= text->buffer.codeUnits.count && index <= rangeEnd);

    SBAssert(text->isMutable && isRangeValid);

    if (!text->isMutable) {
        return;
    }

    if (length > 0) {
        AttributeManagerSetAttribute(&text->attributeManager,
            index, length, attributeID, attributeValue);
    }
}

void SBTextRemoveAttribute(SBMutableTextRef text, SBUInteger index, SBUInteger length,
    SBAttributeID attributeID)
{
    SBUInteger rangeEnd = index + length;
    SBBoolean isRangeValid = (rangeEnd <= text->buffer.codeUnits.count && index <= rangeEnd);

    SBAssert(text->isMutable && isRangeValid);

    if (!text->isMutable) {
        return;
    }

    if (length > 0) {
        AttributeManagerRemoveAttribute(&text->attributeManager, index, length, attributeID);
    }
}

#endif
