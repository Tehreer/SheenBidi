/*
 * Copyright (C) 2016 Muhammad Tayyab Akram
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
#include "SBBidiChain.h"
#include "SBBidiLink.h"
#include "SBCharType.h"
#include "SBCharTypeLookup.h"
#include "SBCodepointSequence.h"
#include "SBIsolatingRun.h"
#include "SBLevelRun.h"
#include "SBLog.h"
#include "SBRunQueue.h"
#include "SBStatusStack.h"
#include "SBTypes.h"
#include "SBParagraph.h"

#define SB_MAX(a, b)                    \
(                                       \
 (a) > (b) ? (a) : (b)                  \
)

#define SB_LEVEL_TO_TYPE(l)             \
(                                       \
   ((l) & 1)                            \
 ? SBCharTypeR                          \
 : SBCharTypeL                          \
)

struct _SBParagraphSupport;
typedef struct _SBParagraphSupport _SBParagraphSupport;
typedef _SBParagraphSupport *_SBParagraphSupportRef;

struct _SBParagraphSupport {
    SBCharType *refTypes;
    SBLevel *refLevels;
    SBBidiLink *fixedLinks;
    SBBidiChain bidiChain;
    SBStatusStack statusStack;
    SBRunQueue runQueue;
    SBIsolatingRun isolatingRun;
};

static _SBParagraphSupportRef _SBParagraphSupportAllocate(SBUInteger linkCount);
static void _SBParagraphSupportInitialize(_SBParagraphSupportRef support, SBCharType *types, SBLevel *levels, SBUInteger length);
static void _SBParagraphSupportDeallocate(_SBParagraphSupportRef support);

static SBParagraphRef _SBParagraphAllocate(SBUInteger length);

static SBUInteger _SBDetermineCharTypes(SBCodepointSequenceRef sequence, SBCharType *types, SBUInteger *outLinkCount);
static void _SBPopulateBidiChain(SBBidiChainRef chain, SBBidiLink *links, SBCharType *types, SBUInteger length);

static SBBidiLinkRef _SBSkipIsolatingRun(SBBidiLinkRef skipLink, SBBidiLinkRef breakLink);
static SBLevel _SBDetermineBaseLevel(SBBidiLinkRef skipLink, SBBidiLinkRef breakLink, SBLevel defaultLevel, SBBoolean isIsolate);
static SBLevel _SBDetermineParagraphLevel(SBBidiChainRef chain, SBLevel baseLevel);

static void _SBDetermineLevels(_SBParagraphSupportRef support, SBLevel baseLevel);
static void _SBProcessRun(_SBParagraphSupportRef support, SBLevelRun levelRun, SBBoolean forceFinish);
static void _SBSaveLevels(SBBidiChainRef chain, SBLevel *levels, SBLevel baseLevel);

static _SBParagraphSupportRef _SBParagraphSupportAllocate(SBUInteger linkCount)
{
    const SBUInteger sizeSupport = sizeof(_SBParagraphSupport);
    const SBUInteger sizeLinks   = sizeof(SBBidiLink) * (linkCount + 1);

    const SBUInteger sizeMemory  = sizeSupport
                                 + sizeLinks;

    SBUInt8 *memory = (SBUInt8 *)malloc(sizeMemory);

    SBUInteger offset = 0;
    _SBParagraphSupportRef support = (_SBParagraphSupportRef)(memory + offset);

    offset += sizeSupport;
    support->fixedLinks = (SBBidiLink *)(memory + offset);

    return support;
}

static void _SBParagraphSupportInitialize(_SBParagraphSupportRef support, SBCharType *types, SBLevel *levels, SBUInteger length)
{
    support->refTypes = types;
    support->refLevels = levels;

    SBBidiChainInitialize(&support->bidiChain);
    SBStatusStackInitialize(&support->statusStack);
    SBRunQueueInitialize(&support->runQueue);
    SBIsolatingRunInitialize(&support->isolatingRun);

    _SBPopulateBidiChain(&support->bidiChain, support->fixedLinks, types, length);
}

static void _SBParagraphSupportDeallocate(_SBParagraphSupportRef support)
{
    SBStatusStackFinalize(&support->statusStack);
    SBRunQueueFinalize(&support->runQueue);
    SBIsolatingRunFinalize(&support->isolatingRun);
    free(support);
}

static SBParagraphRef _SBParagraphAllocate(SBUInteger length)
{
    const SBUInteger sizeParagraph = sizeof(SBParagraph);
    const SBUInteger sizeLevels    = sizeof(SBLevel) * length;
    const SBUInteger sizeMemory    = sizeParagraph
                                   + sizeLevels;

    SBUInt8 *memory = (SBUInt8 *)malloc(sizeMemory);

    SBUInteger offset = 0;
    SBParagraphRef paragraph = (SBParagraphRef)(memory + offset);

    offset += sizeParagraph;
    paragraph->fixedLevels = (SBLevel *)(memory + offset);

    return paragraph;
}

static SBUInteger _SBDetermineBoundary(SBAlgorithmRef algorithm, SBUInteger paragraphOffset, SBUInteger suggestedLength, SBUInteger *outLinkCount)
{
    SBCodepointSequenceRef codepointSequence = algorithm->codepointSequence;
    SBCharType *charTypes = algorithm->fixedTypes;
    SBCharType currentType;
    SBUInteger linkCount;
    SBUInteger index;

    currentType = SBCharTypeNil;
    linkCount = 0;

    for (index = paragraphOffset; index < suggestedLength; index++) {
        SBCharType priorType = currentType;

        currentType = charTypes[index];

        switch (currentType) {
            case SBCharTypeON:
            case SBCharTypeLRE:
            case SBCharTypeRLE:
            case SBCharTypeLRO:
            case SBCharTypeRLO:
            case SBCharTypePDF:
            case SBCharTypeLRI:
            case SBCharTypeRLI:
            case SBCharTypeFSI:
            case SBCharTypePDI:
                ++linkCount;
                break;

            case SBCharTypeB:
                ++linkCount;
                goto Return;

            default:
                if (currentType != priorType) {
                    ++linkCount;
                }
                break;
        }
    }

Return:
    *outLinkCount = linkCount;
    index += SBAlgorithmDetermineSeparatorLength(algorithm, index);

    return (index - paragraphOffset);
}

static void _SBPopulateBidiChain(SBBidiChainRef chain, SBBidiLink *links, SBCharType *types, SBUInteger length)
{
    SBBidiLinkRef priorLink;
    SBBidiLinkRef newLink;
    SBCharType type;
    SBUInteger index;

    priorLink = links;
    newLink = links;
    type = SBCharTypeNil;

#define _SB_ADD_CONSECUTIVE_LINK(t)                     \
{                                                       \
    priorLink->length = index - priorLink->offset;      \
    priorLink = newLink;                                \
                                                        \
    newLink->offset = index;                            \
    newLink->type = t;                                  \
                                                        \
    SBBidiChainAddLink(chain, newLink++);               \
}

    for (index = 0; index < length; index++) {
        SBCharType priorType = type;
        type = types[index];

        switch (type) {
        case SBCharTypeB:
        case SBCharTypeON:
        case SBCharTypeLRE:
        case SBCharTypeRLE:
        case SBCharTypeLRO:
        case SBCharTypeRLO:
        case SBCharTypePDF:
        case SBCharTypeLRI:
        case SBCharTypeRLI:
        case SBCharTypeFSI:
        case SBCharTypePDI:
            _SB_ADD_CONSECUTIVE_LINK(type);

            if (type == SBCharTypeB) {
                index = length;
                goto AddLast;
            }
            break;

        default:
            if (type != priorType) {
                _SB_ADD_CONSECUTIVE_LINK(type);
            }
            break;
        }
    }

AddLast:
    _SB_ADD_CONSECUTIVE_LINK(SBCharTypeNil);

#undef _SB_ADD_CONSECUTIVE_LINK
}

static SBBidiLinkRef _SBSkipIsolatingRun(SBBidiLinkRef skipLink, SBBidiLinkRef breakLink)
{
    SBBidiLinkRef link;
    SBUInteger depth = 1;

    for (link = skipLink->next; link != breakLink; link = link->next) {
        SBCharType type = link->type;

        switch (type) {
        case SBCharTypeLRI:
        case SBCharTypeRLI:
        case SBCharTypeFSI:
            ++depth;
            break;

        case SBCharTypePDI:
            if (--depth == 0) {
                return link;
            }
            break;
        }
    }

    return NULL;
}

static SBLevel _SBDetermineBaseLevel(SBBidiLinkRef skipLink, SBBidiLinkRef breakLink, SBLevel defaultLevel, SBBoolean isIsolate)
{
    SBBidiLinkRef link;

    /* Rules P2, P3 */
    for (link = skipLink->next; link != breakLink; link = link->next) {
        SBCharType type = link->type;

        switch (type) {
        case SBCharTypeL:
            return 0;

        case SBCharTypeAL:
        case SBCharTypeR:
            return 1;

        case SBCharTypeLRI:
        case SBCharTypeRLI:
        case SBCharTypeFSI:
            link = _SBSkipIsolatingRun(link, breakLink);
            if (!link) {
                goto Default;
            }
            break;

        case SBCharTypePDI:
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
        return _SBDetermineBaseLevel(chain->rollerLink, chain->rollerLink,
                                    (baseLevel != SBLevelDefaultRTL ? 0 : 1),
                                    SBFalse);
    }

    return baseLevel;
}

static void _SBDetermineLevels(_SBParagraphSupportRef support, SBLevel baseLevel)
{
    SBBidiChainRef chain = &support->bidiChain;
    SBStatusStackRef stack = &support->statusStack;
    SBBidiLinkRef roller = chain->rollerLink;
    SBBidiLinkRef link;

    SBBidiLinkRef priorLink;
    SBBidiLinkRef firstLink;
    SBBidiLinkRef lastLink;

    SBLevel priorLevel;
    SBCharType sor;
    SBCharType eor;

    SBUInteger overIsolate;
    SBUInteger overEmbedding;
    SBUInteger validIsolate;

    priorLink = roller;
    firstLink = NULL;
    lastLink = NULL;

    priorLevel = baseLevel;
    sor = SBCharTypeNil;

    /* Rule X1 */
    overIsolate = 0;
    overEmbedding = 0;
    validIsolate = 0;

    SBStatusStackPush(stack, baseLevel, SBCharTypeON, SBFalse);

    for (link = roller->next; link != roller; link = link->next) {
        SBBoolean forceFinish = SBFalse;
        SBBoolean bnEquivalent = SBFalse;
        SBCharType type;

        type = link->type;

#define SB_LEAST_GREATER_ODD_LEVEL()                                        \
(                                                                           \
        (SBStatusStackGetEmbeddingLevel(stack) + 1) | 1                     \
)

#define SB_LEAST_GREATER_EVEN_LEVEL()                                       \
(                                                                           \
        (SBStatusStackGetEmbeddingLevel(stack) + 2) & ~1                    \
)

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
        SBLevel newLevel;                                                   \
                                                                            \
        link->level = SBStatusStackGetEmbeddingLevel(stack);                \
        newLevel = l;                                                       \
                                                                            \
        if (newLevel <= SBLevelMax && !overIsolate && !overEmbedding) {     \
            ++validIsolate;                                                 \
            SBStatusStackPush(stack, newLevel, o, SBTrue);                  \
        } else {                                                            \
            ++overIsolate;                                                  \
        }                                                                   \
}

        switch (type) {
        /* Rule X2 */
        case SBCharTypeRLE:
            SB_PUSH_EMBEDDING(SB_LEAST_GREATER_ODD_LEVEL(), SBCharTypeON);
            break;

        /* Rule X3 */
        case SBCharTypeLRE:
            SB_PUSH_EMBEDDING(SB_LEAST_GREATER_EVEN_LEVEL(), SBCharTypeON);
            break;

        /* Rule X4 */
        case SBCharTypeRLO:
            SB_PUSH_EMBEDDING(SB_LEAST_GREATER_ODD_LEVEL(), SBCharTypeR);
            break;

        /* Rule X5 */
        case SBCharTypeLRO:
            SB_PUSH_EMBEDDING(SB_LEAST_GREATER_EVEN_LEVEL(), SBCharTypeL);
            break;

        /* Rule X5a */
        case SBCharTypeRLI:
            SB_PUSH_ISOLATE(SB_LEAST_GREATER_ODD_LEVEL(), SBCharTypeON);
            break;

        /* Rule X5b */
        case SBCharTypeLRI:
            SB_PUSH_ISOLATE(SB_LEAST_GREATER_EVEN_LEVEL(), SBCharTypeON);
            break;

        /* Rule X5c */
        case SBCharTypeFSI:
        {
            SBBoolean isRTL = (_SBDetermineBaseLevel(link, roller, 0, SBTrue) == 1);
            SB_PUSH_ISOLATE(isRTL
                             ? SB_LEAST_GREATER_ODD_LEVEL()
                             : SB_LEAST_GREATER_EVEN_LEVEL(),
                            SBCharTypeON);
            break;
        }

        /* Rule X6 */
        default:
            link->level = SBStatusStackGetEmbeddingLevel(stack);

            if (SBStatusStackGetOverrideStatus(stack) != SBCharTypeON) {
                link->type = SBStatusStackGetOverrideStatus(stack);

                if (priorLink->type == link->type
                    && priorLink->level == link->level) {
                    /*
                     * Properties of this link are same as previous link, therefore merge it and
                     * continue the loop.
                     */
                    SBBidiLinkMergeNext(priorLink);
                    continue;
                }
            }
            break;

        /* Rule X6a */
        case SBCharTypePDI:
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

            link->level = SBStatusStackGetEmbeddingLevel(stack);
            break;

        /* Rule X7 */
        case SBCharTypePDF:
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
        case SBCharTypeB:
            /*
             * These values are reset for clarity, in this implementation B can only occur as the
             * last code in the array.
             */
            SBStatusStackSetEmpty(stack);
            SBStatusStackPush(stack, baseLevel, SBCharTypeON, SBFalse);

            overIsolate = 0;
            overEmbedding = 0;
            validIsolate = 0;

            link->level = baseLevel;
            break;

        case SBCharTypeBN:
            bnEquivalent = SBTrue;
            break;

        case SBCharTypeNil:
            forceFinish = SBTrue;
            link->level = baseLevel;
            break;
        }

        /* Rule X9 */
        if (bnEquivalent) {
            /* The type of this link is BN equivalent, so abandon it and continue the loop. */
            SBBidiLinkAbandonNext(priorLink);
            continue;
        }

        if (sor == SBCharTypeNil) {
            sor = SB_LEVEL_TO_TYPE(SB_MAX(baseLevel, link->level));
            firstLink = link;
            priorLevel = link->level;
        } else if (priorLevel != link->level || forceFinish) {
            SBLevelRun levelRun;
            SBLevel currentLevel;

            /* Since the level has changed at this link, therefore the run must end at prior link. */
            lastLink = priorLink;

            /* Save the current level i.e. level of the next run. */
            currentLevel = link->level;
            /*
             * Now we have both the prior level and the current level i.e. unchanged levels of both
             * the current run and the next run. So, identify eor of the current run.
             * NOTE:
             *      sor of the run has been already determined at this stage.
             */
            eor = SB_LEVEL_TO_TYPE(SB_MAX(priorLevel, currentLevel));

            SBLevelRunInitialize(&levelRun, firstLink, lastLink, sor, eor);
            _SBProcessRun(support, levelRun, forceFinish);

            /* The sor of next run (if any) should be technically equal to eor of this run. */
            sor = eor;
            /* The next run (if any) will start from this index. */
            firstLink = link;

            priorLevel = currentLevel;
        }

        priorLink = link;
    };
}

static void _SBProcessRun(_SBParagraphSupportRef support, SBLevelRun levelRun, SBBoolean forceFinish)
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
    SBBidiLinkRef roller = chain->rollerLink;
    SBBidiLinkRef link;

    SBUInteger index = 0;
    SBLevel level = baseLevel;

    for (link = roller->next; link != roller; link = link->next) {
        SBUInteger offset = link->offset;

        for (; index < offset; index++) {
            levels[index] = level;
        }

        level = link->level;
    }
}

SB_INTERNAL SBParagraphRef SBParagraphCreate(SBAlgorithmRef algorithm,
    SBUInteger paragraphOffset, SBUInteger suggestedLength, SBLevel baseLevel)
{
    SBCodepointSequenceRef codepointSequence = algorithm->codepointSequence;
    SBUInteger bufferLength = codepointSequence->length;
    SBUInteger actualLength;
    SBUInteger linkCount;

    SBParagraphRef paragraph;
    _SBParagraphSupportRef support;
    SBLevel resolvedLevel;

    /* The given range MUST be valid. */
    SBAssert(SBUIntegerVerifyRange(bufferLength, paragraphOffset, suggestedLength) && suggestedLength > 0);

    SBAlgorithmRetain(algorithm);

    SB_LOG_BLOCK_OPENER("Paragraph Input");
    SB_LOG_STATEMENT("Paragraph Offset", 1, SB_LOG_NUMBER(paragraphOffset));
    SB_LOG_STATEMENT("Suggested Length", 1, SB_LOG_NUMBER(suggestedLength));
    SB_LOG_STATEMENT("Base Direction",   1, SB_LOG_BASE_LEVEL(baseLevel));
    SB_LOG_BLOCK_CLOSER();

    actualLength = _SBDetermineBoundary(algorithm, paragraphOffset, suggestedLength, &linkCount);

    SB_LOG_BLOCK_OPENER("Determined Paragraph Boundary");
    SB_LOG_STATEMENT("Actual Length", 1, SB_LOG_NUMBER(actualLength));
    SB_LOG_BLOCK_CLOSER();

    paragraph = _SBParagraphAllocate(actualLength);
    paragraph->refTypes = algorithm->fixedTypes + paragraphOffset;

    support = _SBParagraphSupportAllocate(linkCount + 1);
    _SBParagraphSupportInitialize(support, paragraph->refTypes, paragraph->fixedLevels, actualLength);
    
    resolvedLevel = _SBDetermineParagraphLevel(&support->bidiChain, baseLevel);
    
    SB_LOG_BLOCK_OPENER("Determined Paragraph Level");
    SB_LOG_STATEMENT("Base Level", 1, SB_LOG_LEVEL(resolvedLevel));
    SB_LOG_BLOCK_CLOSER();

    support->isolatingRun.codepointSequence = codepointSequence;
    support->isolatingRun.paragraphLevel = resolvedLevel;
    
    _SBDetermineLevels(support, resolvedLevel);
    _SBSaveLevels(&support->bidiChain, support->refLevels, resolvedLevel);
    
    SB_LOG_BLOCK_OPENER("Determined Embedding Levels");
    SB_LOG_STATEMENT("Levels",  1, SB_LOG_LEVELS_ARRAY(paragraph->fixedLevels, actualLength));
    SB_LOG_BLOCK_CLOSER();

    paragraph->algorithm = algorithm;
    paragraph->offset = 0;
    paragraph->length = actualLength;
    paragraph->baseLevel = resolvedLevel;
    paragraph->_retainCount = 1;
    
    _SBParagraphSupportDeallocate(support);
    
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
