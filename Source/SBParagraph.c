/*
 * Copyright (C) 2014-2018 Muhammad Tayyab Akram
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

#include <SBConfig.h>
#include <stddef.h>
#include <stdlib.h>

#include "SBAlgorithm.h"
#include "SBAssert.h"
#include "SBBase.h"
#include "SBBidiChain.h"
#include "SBBidiTypeLookup.h"
#include "SBCodepointSequence.h"
#include "SBIsolatingRun.h"
#include "SBLevelRun.h"
#include "SBLine.h"
#include "SBLog.h"
#include "SBRunQueue.h"
#include "SBStatusStack.h"
#include "SBParagraph.h"

#define SB_MAX(a, b)                    \
(                                       \
 (a) > (b) ? (a) : (b)                  \
)

#define SB_LEVEL_TO_TYPE(l)             \
(                                       \
   ((l) & 1)                            \
 ? SBBidiTypeR                          \
 : SBBidiTypeL                          \
)

typedef struct _SBParagraphSupport {
    SBBidiChain bidiChain;
    SBStatusStack statusStack;
    SBRunQueue runQueue;
    SBIsolatingRun isolatingRun;
} _SBParagraphSupport, *_SBParagraphSupportRef;

static void _SBPopulateBidiChain(SBBidiChainRef chain, SBBidiType *types, SBUInteger length);
static void _SBProcessRun(_SBParagraphSupportRef support, const SBLevelRunRef levelRun, SBBoolean forceFinish);

static _SBParagraphSupportRef _SBParagraphSupportCreate(SBBidiType *types, SBLevel *levels, SBUInteger length)
{
    const SBUInteger sizeSupport = sizeof(_SBParagraphSupport);
    const SBUInteger sizeLinks   = sizeof(SBBidiLink) * (length + 2);
    const SBUInteger sizeTypes   = sizeof(SBBidiType) * (length + 2);
    const SBUInteger sizeMemory  = sizeSupport + sizeLinks + sizeTypes;

    const SBUInteger offsetSupport = 0;
    const SBUInteger offsetLinks   = offsetSupport + sizeSupport;
    const SBUInteger offsetTypes   = offsetLinks + sizeLinks;

    SBUInt8 *memory = (SBUInt8 *)malloc(sizeMemory);
    _SBParagraphSupportRef support = (_SBParagraphSupportRef)(memory + offsetSupport);
    SBBidiLink *fixedLinks = (SBBidiLink *)(memory + offsetLinks);
    SBBidiType *fixedTypes = (SBBidiType *)(memory + offsetTypes);

    SBBidiChainInitialize(&support->bidiChain, fixedTypes, levels, fixedLinks);
    SBStatusStackInitialize(&support->statusStack);
    SBRunQueueInitialize(&support->runQueue);
    SBIsolatingRunInitialize(&support->isolatingRun);

    _SBPopulateBidiChain(&support->bidiChain, types, length);

    return support;
}

static void _SBParagraphSupportDestroy(_SBParagraphSupportRef support)
{
    SBStatusStackFinalize(&support->statusStack);
    SBRunQueueFinalize(&support->runQueue);
    SBIsolatingRunFinalize(&support->isolatingRun);
    free(support);
}

static SBParagraphRef _SBParagraphAllocate(SBUInteger length)
{
    const SBUInteger sizeParagraph = sizeof(SBParagraph);
    const SBUInteger sizeLevels    = sizeof(SBLevel) * (length + 2);
    const SBUInteger sizeMemory    = sizeParagraph + sizeLevels;

    const SBUInteger offsetParagraph = 0;
    const SBUInteger offsetLevels    = offsetParagraph + sizeParagraph;

    SBUInt8 *memory = (SBUInt8 *)malloc(sizeMemory);
    SBParagraphRef paragraph = (SBParagraphRef)(memory + offsetParagraph);
    SBLevel *levels = (SBLevel *)(memory + offsetLevels);

    paragraph->fixedLevels = levels;

    return paragraph;
}

static SBUInteger _SBDetermineBoundary(SBAlgorithmRef algorithm, SBUInteger paragraphOffset, SBUInteger suggestedLength)
{
    SBBidiType *bidiTypes = algorithm->fixedTypes;
    SBUInteger suggestedLimit = paragraphOffset + suggestedLength;
    SBUInteger stringIndex;

    for (stringIndex = paragraphOffset; stringIndex < suggestedLimit; stringIndex++) {
        if (bidiTypes[stringIndex] == SBBidiTypeB) {
            goto Return;
        }
    }

Return:
    stringIndex += SBAlgorithmGetSeparatorLength(algorithm, stringIndex);

    return (stringIndex - paragraphOffset);
}

static void _SBPopulateBidiChain(SBBidiChainRef chain, SBBidiType *types, SBUInteger length)
{
    SBBidiType type = SBBidiTypeNil;
    SBUInteger priorIndex = SBInvalidIndex;
    SBUInteger index;

    for (index = 0; index < length; index++) {
        SBBidiType priorType = type;
        type = types[index];

        switch (type) {
        case SBBidiTypeB:
        case SBBidiTypeON:
        case SBBidiTypeLRE:
        case SBBidiTypeRLE:
        case SBBidiTypeLRO:
        case SBBidiTypeRLO:
        case SBBidiTypePDF:
        case SBBidiTypeLRI:
        case SBBidiTypeRLI:
        case SBBidiTypeFSI:
        case SBBidiTypePDI:
            SBBidiChainAdd(chain, type, index - priorIndex);
            priorIndex = index;

            if (type == SBBidiTypeB) {
                index = length;
                goto AddLast;
            }
            break;

        default:
            if (type != priorType) {
                SBBidiChainAdd(chain, type, index - priorIndex);
                priorIndex = index;
            }
            break;
        }
    }

AddLast:
    SBBidiChainAdd(chain, SBBidiTypeNil, index - priorIndex);
}

static SBBidiLink _SBSkipIsolatingRun(SBBidiChainRef chain, SBBidiLink skipLink, SBBidiLink breakLink)
{
    SBBidiLink link = skipLink;
    SBUInteger depth = 1;

    while ((link = SBBidiChainGetNext(chain, link)) != breakLink) {
        SBBidiType type = SBBidiChainGetType(chain, link);

        switch (type) {
        case SBBidiTypeLRI:
        case SBBidiTypeRLI:
        case SBBidiTypeFSI:
            ++depth;
            break;

        case SBBidiTypePDI:
            if (--depth == 0) {
                return link;
            }
            break;
        }
    }

    return SBBidiLinkNone;
}

static SBLevel _SBDetermineBaseLevel(SBBidiChainRef chain, SBBidiLink skipLink, SBBidiLink breakLink, SBLevel defaultLevel, SBBoolean isIsolate)
{
    SBBidiLink link = skipLink;

    /* Rules P2, P3 */
    while ((link = SBBidiChainGetNext(chain, link)) != breakLink) {
        SBBidiType type = SBBidiChainGetType(chain, link);

        switch (type) {
        case SBBidiTypeL:
            return 0;

        case SBBidiTypeAL:
        case SBBidiTypeR:
            return 1;

        case SBBidiTypeLRI:
        case SBBidiTypeRLI:
        case SBBidiTypeFSI:
            link = _SBSkipIsolatingRun(chain, link, breakLink);
            if (link == SBBidiLinkNone) {
                goto Default;
            }
            break;

        case SBBidiTypePDI:
            if (isIsolate) {
                /*
                 * In case of isolating run, the PDI will be the last code point.
                 * NOTE:
                 *      The inner isolating runs will be skipped by the case above this one.
                 */
                goto Default;
            }
            break;
        }
    }

Default:
    return defaultLevel;
}

static SBLevel _SBDetermineParagraphLevel(SBBidiChainRef chain, SBLevel baseLevel)
{
    if (baseLevel >= SBLevelMax) {
        return _SBDetermineBaseLevel(chain, chain->roller, chain->roller,
                                     (baseLevel != SBLevelDefaultRTL ? 0 : 1),
                                     SBFalse);
    }

    return baseLevel;
}

static void _SBDetermineLevels(_SBParagraphSupportRef support, SBLevel baseLevel)
{
    SBBidiChainRef chain = &support->bidiChain;
    SBStatusStackRef stack = &support->statusStack;
    SBBidiLink roller = chain->roller;
    SBBidiLink link;

    SBBidiLink priorLink;
    SBBidiLink firstLink;
    SBBidiLink lastLink;

    SBLevel priorLevel;
    SBBidiType sor;
    SBBidiType eor;

    SBUInteger overIsolate;
    SBUInteger overEmbedding;
    SBUInteger validIsolate;

    priorLink = chain->roller;
    firstLink = SBBidiLinkNone;
    lastLink = SBBidiLinkNone;

    priorLevel = baseLevel;
    sor = SBBidiTypeNil;

    /* Rule X1 */
    overIsolate = 0;
    overEmbedding = 0;
    validIsolate = 0;

    SBStatusStackPush(stack, baseLevel, SBBidiTypeON, SBFalse);

    SBBidiChainForEach(chain, roller, link) {
        SBBoolean forceFinish = SBFalse;
        SBBoolean bnEquivalent = SBFalse;
        SBBidiType type;

        type = SBBidiChainGetType(chain, link);

#define SB_LEAST_GREATER_ODD_LEVEL()                                        \
(                                                                           \
        (SBStatusStackGetEmbeddingLevel(stack) + 1) | 1                     \
)

#define SB_LEAST_GREATER_EVEN_LEVEL()                                       \
(                                                                           \
        (SBStatusStackGetEmbeddingLevel(stack) + 2) & ~1                    \
)

#define SB_MERGE_LINK_IF_NEEDED()                                           \
{                                                                           \
        if (SBBidiChainMergeIfEqual(chain, priorLink, link)) {              \
            continue;                                                       \
        }                                                                   \
}

#define SB_PUSH_EMBEDDING(l, o)                                             \
{                                                                           \
        SBLevel newLevel;                                                   \
                                                                            \
        bnEquivalent = SBTrue;                                              \
        newLevel = l;                                                       \
                                                                            \
        if (newLevel <= SBLevelMax && !overIsolate && !overEmbedding) {     \
            SBStatusStackPush(stack, newLevel, o, SBFalse);                 \
        } else {                                                            \
            if (!overIsolate) {                                             \
                ++overEmbedding;                                            \
            }                                                               \
        }                                                                   \
}

#define SB_PUSH_ISOLATE(l, o)                                               \
{                                                                           \
        SBBidiType priorStatus = SBStatusStackGetOverrideStatus(stack);     \
        SBLevel newLevel = l;                                               \
                                                                            \
        SBBidiChainSetLevel(chain, link,                                    \
                            SBStatusStackGetEmbeddingLevel(stack));         \
                                                                            \
        if (newLevel <= SBLevelMax && !overIsolate && !overEmbedding) {     \
            ++validIsolate;                                                 \
            SBStatusStackPush(stack, newLevel, o, SBTrue);                  \
        } else {                                                            \
            ++overIsolate;                                                  \
        }                                                                   \
                                                                            \
        if (priorStatus != SBBidiTypeON) {                                  \
            SBBidiChainSetType(chain, link, priorStatus);                   \
            SB_MERGE_LINK_IF_NEEDED();                                      \
        }                                                                   \
}

        switch (type) {
        /* Rule X2 */
        case SBBidiTypeRLE:
            SB_PUSH_EMBEDDING(SB_LEAST_GREATER_ODD_LEVEL(), SBBidiTypeON);
            break;

        /* Rule X3 */
        case SBBidiTypeLRE:
            SB_PUSH_EMBEDDING(SB_LEAST_GREATER_EVEN_LEVEL(), SBBidiTypeON);
            break;

        /* Rule X4 */
        case SBBidiTypeRLO:
            SB_PUSH_EMBEDDING(SB_LEAST_GREATER_ODD_LEVEL(), SBBidiTypeR);
            break;

        /* Rule X5 */
        case SBBidiTypeLRO:
            SB_PUSH_EMBEDDING(SB_LEAST_GREATER_EVEN_LEVEL(), SBBidiTypeL);
            break;

        /* Rule X5a */
        case SBBidiTypeRLI:
            SB_PUSH_ISOLATE(SB_LEAST_GREATER_ODD_LEVEL(), SBBidiTypeON);
            break;

        /* Rule X5b */
        case SBBidiTypeLRI:
            SB_PUSH_ISOLATE(SB_LEAST_GREATER_EVEN_LEVEL(), SBBidiTypeON);
            break;

        /* Rule X5c */
        case SBBidiTypeFSI:
        {
            SBBoolean isRTL = (_SBDetermineBaseLevel(chain, link, roller, 0, SBTrue) == 1);
            SB_PUSH_ISOLATE(isRTL
                             ? SB_LEAST_GREATER_ODD_LEVEL()
                             : SB_LEAST_GREATER_EVEN_LEVEL(),
                            SBBidiTypeON);
            break;
        }

        /* Rule X6 */
        default:
            SBBidiChainSetLevel(chain, link, SBStatusStackGetEmbeddingLevel(stack));

            if (SBStatusStackGetOverrideStatus(stack) != SBBidiTypeON) {
                SBBidiChainSetType(chain, link, SBStatusStackGetOverrideStatus(stack));
                SB_MERGE_LINK_IF_NEEDED();
            }
            break;

        /* Rule X6a */
        case SBBidiTypePDI:
        {
            SBBidiType overrideStatus;

            if (overIsolate != 0) {
                --overIsolate;
            } else if (validIsolate == 0) {
                /* Do nothing */
            } else {
                overEmbedding = 0;

                while (!SBStatusStackGetIsolateStatus(stack)) {
                    SBStatusStackPop(stack);
                };
                SBStatusStackPop(stack);

                --validIsolate;
            }

            SBBidiChainSetLevel(chain, link, SBStatusStackGetEmbeddingLevel(stack));
            overrideStatus = SBStatusStackGetOverrideStatus(stack);

            if (overrideStatus != SBBidiTypeON) {
                SBBidiChainSetType(chain, link, overrideStatus);
                SB_MERGE_LINK_IF_NEEDED();
            }
            break;
        }

        /* Rule X7 */
        case SBBidiTypePDF:
            bnEquivalent = SBTrue;

            if (overIsolate != 0) {
                /* do nothing */
            } else if (overEmbedding != 0) {
                --overEmbedding;
            } else if (!SBStatusStackGetIsolateStatus(stack) && stack->count >= 2) {
                SBStatusStackPop(stack);
            }
            break;

        /* Rule X8 */
        case SBBidiTypeB:
            /*
             * These values are reset for clarity, in this implementation B can only occur as the
             * last code in the array.
             */
            SBStatusStackSetEmpty(stack);
            SBStatusStackPush(stack, baseLevel, SBBidiTypeON, SBFalse);

            overIsolate = 0;
            overEmbedding = 0;
            validIsolate = 0;

            SBBidiChainSetLevel(chain, link, baseLevel);
            break;

        case SBBidiTypeBN:
            bnEquivalent = SBTrue;
            break;

        case SBBidiTypeNil:
            forceFinish = SBTrue;
            SBBidiChainSetLevel(chain, link, baseLevel);
            break;
        }

        /* Rule X9 */
        if (bnEquivalent) {
            /* The type of this link is BN equivalent, so abandon it and continue the loop. */
            SBBidiChainSetType(chain, link, SBBidiTypeBN);
            SBBidiChainAbandonNext(chain, priorLink);
            continue;
        }

        if (sor == SBBidiTypeNil) {
            sor = SB_LEVEL_TO_TYPE(SB_MAX(baseLevel, SBBidiChainGetLevel(chain, link)));
            firstLink = link;
            priorLevel = SBBidiChainGetLevel(chain, link);
        } else if (priorLevel != SBBidiChainGetLevel(chain, link) || forceFinish) {
            SBLevelRun levelRun;
            SBLevel currentLevel;

            /* Since the level has changed at this link, therefore the run must end at prior link. */
            lastLink = priorLink;

            /* Save the current level i.e. level of the next run. */
            currentLevel = SBBidiChainGetLevel(chain, link);
            /*
             * Now we have both the prior level and the current level i.e. unchanged levels of both
             * the current run and the next run. So, identify eor of the current run.
             * NOTE:
             *      sor of the run has been already determined at this stage.
             */
            eor = SB_LEVEL_TO_TYPE(SB_MAX(priorLevel, currentLevel));

            SBLevelRunInitialize(&levelRun, chain, firstLink, lastLink, sor, eor);
            _SBProcessRun(support, &levelRun, forceFinish);

            /* The sor of next run (if any) should be technically equal to eor of this run. */
            sor = eor;
            /* The next run (if any) will start from this index. */
            firstLink = link;

            priorLevel = currentLevel;
        }

        priorLink = link;
    };
}

static void _SBProcessRun(_SBParagraphSupportRef support, const SBLevelRunRef levelRun, SBBoolean forceFinish)
{
    SBRunQueueRef queue = &support->runQueue;
    SBRunQueueEnqueue(queue, levelRun);

    if (queue->shouldDequeue || forceFinish) {
        SBIsolatingRunRef isolatingRun = &support->isolatingRun;
        SBLevelRunRef peek;

        /* Rule X10 */
        for (; queue->count != 0; SBRunQueueDequeue(queue)) {
            peek = queue->peek;
            if (SBRunKindIsAttachedTerminating(peek->kind)) {
                continue;
            }

            isolatingRun->baseLevelRun = peek;
            SBIsolatingRunResolve(isolatingRun);
        }
    }
}

static void _SBSaveLevels(SBBidiChainRef chain, SBLevel *levels, SBLevel baseLevel)
{
    SBBidiLink roller = chain->roller;
    SBBidiLink link;

    SBUInteger index = 0;
    SBLevel level = baseLevel;

    SBBidiChainForEach(chain, roller, link) {
        SBUInteger offset = SBBidiChainGetOffset(chain, link);

        for (; index < offset; index++) {
            levels[index] = level;
        }

        level = SBBidiChainGetLevel(chain, link);
    }
}

SB_INTERNAL SBParagraphRef SBParagraphCreate(SBAlgorithmRef algorithm,
    SBUInteger paragraphOffset, SBUInteger suggestedLength, SBLevel baseLevel)
{
    const SBCodepointSequence *codepointSequence = &algorithm->codepointSequence;
    SBUInteger stringLength = codepointSequence->stringLength;
    SBUInteger actualLength;

    SBParagraphRef paragraph;
    _SBParagraphSupportRef support;
    SBLevel resolvedLevel;

    /* The given range MUST be valid. */
    SBAssert(SBUIntegerVerifyRange(stringLength, paragraphOffset, suggestedLength) && suggestedLength > 0);

    SB_LOG_BLOCK_OPENER("Paragraph Input");
    SB_LOG_STATEMENT("Paragraph Offset", 1, SB_LOG_NUMBER(paragraphOffset));
    SB_LOG_STATEMENT("Suggested Length", 1, SB_LOG_NUMBER(suggestedLength));
    SB_LOG_STATEMENT("Base Direction",   1, SB_LOG_BASE_LEVEL(baseLevel));
    SB_LOG_BLOCK_CLOSER();

    actualLength = _SBDetermineBoundary(algorithm, paragraphOffset, suggestedLength);

    SB_LOG_BLOCK_OPENER("Determined Paragraph Boundary");
    SB_LOG_STATEMENT("Actual Length", 1, SB_LOG_NUMBER(actualLength));
    SB_LOG_BLOCK_CLOSER();

    paragraph = _SBParagraphAllocate(actualLength);
    paragraph->refTypes = algorithm->fixedTypes + paragraphOffset;

    support = _SBParagraphSupportCreate(paragraph->refTypes, paragraph->fixedLevels, actualLength);
    
    resolvedLevel = _SBDetermineParagraphLevel(&support->bidiChain, baseLevel);
    
    SB_LOG_BLOCK_OPENER("Determined Paragraph Level");
    SB_LOG_STATEMENT("Base Level", 1, SB_LOG_LEVEL(resolvedLevel));
    SB_LOG_BLOCK_CLOSER();

    support->isolatingRun.codepointSequence = codepointSequence;
    support->isolatingRun.paragraphOffset = paragraphOffset;
    support->isolatingRun.bidiTypes = paragraph->refTypes;
    support->isolatingRun.bidiChain = &support->bidiChain;
    support->isolatingRun.paragraphLevel = resolvedLevel;
    
    _SBDetermineLevels(support, resolvedLevel);
    _SBSaveLevels(&support->bidiChain, ++paragraph->fixedLevels, resolvedLevel);
    
    SB_LOG_BLOCK_OPENER("Determined Embedding Levels");
    SB_LOG_STATEMENT("Levels",  1, SB_LOG_LEVELS_ARRAY(paragraph->fixedLevels, actualLength));
    SB_LOG_BLOCK_CLOSER();

    paragraph->algorithm = SBAlgorithmRetain(algorithm);
    paragraph->offset = paragraphOffset;
    paragraph->length = actualLength;
    paragraph->baseLevel = resolvedLevel;
    paragraph->_retainCount = 1;
    
    _SBParagraphSupportDestroy(support);
    
    SB_LOG_BREAKER();
    
    return paragraph;
}

SBUInteger SBParagraphGetOffset(SBParagraphRef paragraph)
{
    return paragraph->offset;
}

SBUInteger SBParagraphGetLength(SBParagraphRef paragraph)
{
    return paragraph->length;
}

SBLevel SBParagraphGetBaseLevel(SBParagraphRef paragraph)
{
    return paragraph->baseLevel;
}

const SBLevel *SBParagraphGetLevelsPtr(SBParagraphRef paragraph)
{
    return paragraph->fixedLevels;
}

SBLineRef SBParagraphCreateLine(SBParagraphRef paragraph, SBUInteger lineOffset, SBUInteger lineLength)
{
    SBUInteger paragraphOffset = paragraph->offset;
    SBUInteger paragraphLength = paragraph->length;
    SBUInteger paragraphLimit = paragraphOffset + paragraphLength;
    SBUInteger lineLimit = lineOffset + lineLength;

    if (lineOffset < lineLimit && lineOffset >= paragraphOffset && lineLimit <= paragraphLimit) {
        return SBLineCreate(paragraph, lineOffset, lineLength);
    }

    return NULL;
}

SBParagraphRef SBParagraphRetain(SBParagraphRef paragraph)
{
    if (paragraph) {
        ++paragraph->_retainCount;
    }
    
    return paragraph;
}

void SBParagraphRelease(SBParagraphRef paragraph)
{
    if (paragraph && --paragraph->_retainCount == 0) {
        SBAlgorithmRelease(paragraph->algorithm);
        free(paragraph);
    }
}
