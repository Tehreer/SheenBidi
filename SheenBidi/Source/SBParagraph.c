/*
 * Copyright (C) 2014 Muhammad Tayyab Akram
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

#include <SBConfig.h>
#include <SBTypes.h>
#include <SBBaseDirection.h>

#include "SBAssert.h"
#include "SBLog.h"

#include "SBCharType.h"
#include "SBCharTypeLookup.h"

#include "SBBidiLink.h"
#include "SBBidiChain.h"
#include "SBLevelRun.h"
#include "SBIsolatingRun.h"

#include "SBStatusStack.h"
#include "SBRunQueue.h"

#include "SBParagraph.h"

#define SB_MAX(a, b)                    \
(                                       \
 (a) > (b) ? (a) : (b)                  \
)

#define SB_LEVEL_MAX                    125

#define SB_LEVEL_TO_TYPE(l)             \
(                                       \
   ((l) & 1)                            \
 ? SB_CHAR_TYPE__R                      \
 : SB_CHAR_TYPE__L                      \
)

struct _SBParagraphSupport;
typedef struct _SBParagraphSupport _SBParagraphSupport;
typedef _SBParagraphSupport *_SBParagraphSupportRef;

struct _SBParagraphSupport {
    SBUnichar *refCharacters;
    SBCharType *refTypes;
    SBLevel *refLevels;
    SBBidiLink *fixedLinks;
    SBBidiChain bidiChain;
    SBStatusStack statusStack;
    SBRunQueue runQueue;
    SBIsolatingRun isolatingRun;
};

static _SBParagraphSupportRef _SBParagraphSupportAllocate(SBUInteger linkCount);
static void _SBParagraphSupportInitialize(_SBParagraphSupportRef support, SBUnichar *characters, SBCharType *types, SBLevel *levels, SBUInteger length);
static void _SBParagraphSupportDeallocate(_SBParagraphSupportRef support);

static SBParagraphRef _SBParagraphAllocate(SBUInteger length);

static SBUInteger _SBDetermineCharTypes(SBUnichar *characters, SBCharType *types, SBUInteger length);
static void _SBPopulateBidiChain(SBBidiChainRef chain, SBBidiLink *links, SBCharType *types, SBUInteger length);

static SBBidiLinkRef _SBSkipIsolatingRun(SBBidiLinkRef skipLink, SBBidiLinkRef breakLink);
static SBLevel _SBDetermineBaseLevel(SBBidiLinkRef skipLink, SBBidiLinkRef breakLink, SBLevel defaultLevel, SBBoolean isIsolate);
static SBLevel _SBDetermineParagraphLevel(SBBidiChainRef chain, SBBaseDirection baseDirection);

static void _SBDetermineLevels(_SBParagraphSupportRef support, SBLevel baseLevel);
static void _SBProcessRun(_SBParagraphSupportRef support, SBLevelRun levelRun, SBBoolean forceFinish);
static void _SBSaveLevels(SBBidiChainRef chain, SBLevel *levels, SBLevel baseLevel);

static _SBParagraphSupportRef _SBParagraphSupportAllocate(SBUInteger linkCount) {
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

static void _SBParagraphSupportInitialize(_SBParagraphSupportRef support, SBUnichar *characters, SBCharType *types, SBLevel *levels, SBUInteger length) {
    support->refCharacters = characters;
    support->refTypes = types;
    support->refLevels = levels;

    SBBidiChainInitialize(&support->bidiChain);
    SBStatusStackInitialize(&support->statusStack);
    SBRunQueueInitialize(&support->runQueue);
    SBIsolatingRunInitialize(&support->isolatingRun);

    _SBPopulateBidiChain(&support->bidiChain, support->fixedLinks, types, length);
}

static void _SBParagraphSupportDeallocate(_SBParagraphSupportRef support) {
    SBStatusStackFinalize(&support->statusStack);
    SBRunQueueFinalize(&support->runQueue);
    SBIsolatingRunFinalize(&support->isolatingRun);
    free(support);
}

static SBParagraphRef _SBParagraphAllocate(SBUInteger length) {
    const SBUInteger sizeParagraph = sizeof(SBParagraph);
    const SBUInteger sizeTypes     = sizeof(SBCharType) * length;
    const SBUInteger sizeLevels    = sizeof(SBLevel)     * length;

    const SBUInteger sizeMemory    = sizeParagraph
                                   + sizeTypes
                                   + sizeLevels;

    SBUInt8 *memory = (SBUInt8 *)malloc(sizeMemory);

    SBUInteger offset = 0;
    SBParagraphRef paragraph = (SBParagraphRef)(memory + offset);

    offset += sizeParagraph;
    paragraph->fixedTypes = (SBCharType *)(memory + offset);

    offset += sizeTypes;
    paragraph->fixedLevels = (SBLevel *)(memory + offset);

    return paragraph;
}

static SBUInteger _SBDetermineCharTypes(SBUnichar *characters, SBCharType *types, SBUInteger length) {
    SBUInteger linkCount;
    SBCharType newType;
    SBUInteger index;

    linkCount = 0;
    newType = SB_CHAR_TYPE__NIL;

    for (index = 0; index < length; ++index) {
        SBUnichar character = characters[index];
        SBCharType priorType = newType;

        newType = SBCharTypeDetermineForUnicode(character);

        switch (newType) {
        case SB_CHAR_TYPE__ON:
        case SB_CHAR_TYPE__FORMAT_CASE:
            ++linkCount;
            break;

        default:
            if (newType != priorType) {
                ++linkCount;
            }
            break;
        }

        types[index] = newType;
    }

    return linkCount;
}

static void _SBPopulateBidiChain(SBBidiChainRef chain, SBBidiLink *links, SBCharType *types, SBUInteger length) {
    SBBidiLinkRef priorLink;
    SBBidiLinkRef newLink;
    SBCharType type;
    SBUInteger index;

    priorLink = links;
    newLink = links;
    type = SB_CHAR_TYPE__NIL;

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
        case SB_CHAR_TYPE__ON:
        case SB_CHAR_TYPE__FORMAT_CASE:
            _SB_ADD_CONSECUTIVE_LINK(type);
            break;

        default:
            if (type != priorType) {
                _SB_ADD_CONSECUTIVE_LINK(type);
            }
            break;
        }
    }

    _SB_ADD_CONSECUTIVE_LINK(SB_CHAR_TYPE__NIL);

#undef _SB_ADD_CONSECUTIVE_LINK
}

static SBBidiLinkRef _SBSkipIsolatingRun(SBBidiLinkRef skipLink, SBBidiLinkRef breakLink) {
    SBBidiLinkRef link;
    SBUInteger depth = 1;

    for (link = skipLink->next; link != breakLink; link = link->next) {
        SBCharType type = link->type;

        switch (type) {
        case SB_CHAR_TYPE__ISOLATE_INITIATOR_CASE:
            ++depth;
            break;

        case SB_CHAR_TYPE__PDI:
            if (--depth == 0) {
                return link;
            }
            break;
        }
    }

    return NULL;
}

static SBLevel _SBDetermineBaseLevel(SBBidiLinkRef skipLink, SBBidiLinkRef breakLink, SBLevel defaultLevel, SBBoolean isIsolate) {
    SBBidiLinkRef link;

    /* Rules P2, P3 */
    for (link = skipLink->next; link != breakLink; link = link->next) {
        SBCharType type = link->type;

        switch (type) {
        case SB_CHAR_TYPE__L:
            return 0;

        case SB_CHAR_TYPE__AL:
        case SB_CHAR_TYPE__R:
            return 1;

        case SB_CHAR_TYPE__ISOLATE_INITIATOR_CASE:
            link = _SBSkipIsolatingRun(link, breakLink);
            if (!link) {
                goto Default;
            }
            break;

        case SB_CHAR_TYPE__PDI:
            if (isIsolate) {
                /*
                 * In case of isolating run, the PDI will be the last character.
                 * Note: The inner isolating runs will be skipped by the case
                 *       above this one.
                 */
                goto Default;
            }
            break;
        }
    }

Default:
    return defaultLevel;
}

static SBLevel _SBDetermineParagraphLevel(SBBidiChainRef chain, SBBaseDirection baseDirection) {
    SBLevel level;

    switch (baseDirection) {
    case SBBaseDirectionLTR:
        level = 0;
        break;

    case SBBaseDirectionRTL:
        level = 1;
        break;

    default:
        level = _SBDetermineBaseLevel(chain->rollerLink, chain->rollerLink,
                                      baseDirection == SBBaseDirectionAutoLTR ? 0 : 1,
                                      SBFalse);
        break;
    }

    return level;
}

static void _SBDetermineLevels(_SBParagraphSupportRef support, SBLevel baseLevel) {
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
    sor = SB_CHAR_TYPE__NIL;

    /* Rule X1 */
    overIsolate = 0;
    overEmbedding = 0;
    validIsolate = 0;

    SBStatusStackPush(stack, baseLevel, SB_CHAR_TYPE__ON, SBFalse);

    for (link = roller->next; link != roller; link = link->next) {
        SBBoolean forceFinish;
        SBBoolean bnEquivalent;

        SBCharType type;

        forceFinish = SBFalse;
        bnEquivalent = SBFalse;

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
        if (newLevel <= SB_LEVEL_MAX && !overIsolate && !overEmbedding) {   \
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
        if (newLevel <= SB_LEVEL_MAX && !overIsolate && !overEmbedding) {   \
            ++validIsolate;                                                 \
            SBStatusStackPush(stack, newLevel, o, SBTrue);                  \
        } else {                                                            \
            ++overIsolate;                                                  \
        }                                                                   \
}

        switch (type) {
        /* Rule X2 */
        case SB_CHAR_TYPE__RLE:
            SB_PUSH_EMBEDDING(SB_LEAST_GREATER_ODD_LEVEL(), SB_CHAR_TYPE__ON);
            break;

        /* Rule X3 */
        case SB_CHAR_TYPE__LRE:
            SB_PUSH_EMBEDDING(SB_LEAST_GREATER_EVEN_LEVEL(), SB_CHAR_TYPE__ON);
            break;

        /* Rule X4 */
        case SB_CHAR_TYPE__RLO:
            SB_PUSH_EMBEDDING(SB_LEAST_GREATER_ODD_LEVEL(), SB_CHAR_TYPE__R);
            break;

        /* Rule X5 */
        case SB_CHAR_TYPE__LRO:
            SB_PUSH_EMBEDDING(SB_LEAST_GREATER_EVEN_LEVEL(), SB_CHAR_TYPE__L);
            break;

        /* Rule X5a */
        case SB_CHAR_TYPE__RLI:
            SB_PUSH_ISOLATE(SB_LEAST_GREATER_ODD_LEVEL(), SB_CHAR_TYPE__ON);
            break;

        /* Rule X5b */
        case SB_CHAR_TYPE__LRI:
            SB_PUSH_ISOLATE(SB_LEAST_GREATER_EVEN_LEVEL(), SB_CHAR_TYPE__ON);
            break;

        /* Rule X5c */
        case SB_CHAR_TYPE__FSI:
        {
            SBBoolean isRTL = (_SBDetermineBaseLevel(link, roller, 0, SBTrue) == 1);
            SB_PUSH_ISOLATE(isRTL
                             ? SB_LEAST_GREATER_ODD_LEVEL()
                             : SB_LEAST_GREATER_EVEN_LEVEL(),
                            SB_CHAR_TYPE__ON);
            break;
        }

        /* Rule X6 */
        default:
            link->level = SBStatusStackGetEmbeddingLevel(stack);

            if (SBStatusStackGetOverrideStatus(stack) != SB_CHAR_TYPE__ON) {
                link->type = SBStatusStackGetOverrideStatus(stack);

                if (priorLink->type == link->type
                    && priorLink->level == link->level) {
                    /*
                     * Properties of this link are same as previous link,
                     * therefore merge it and continue the loop.
                     */
                    SBBidiLinkMergeNext(priorLink);
                    continue;
                }
            }
            break;

        /* Rule X6a */
        case SB_CHAR_TYPE__PDI:
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
        case SB_CHAR_TYPE__PDF:
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
        case SB_CHAR_TYPE__B:
            /* These values are reset for clarity, in this implementation B
             * can only occur as the last code in the array.
             */
            SBStatusStackSetEmpty(stack);

            overIsolate = 0;
            overEmbedding = 0;
            validIsolate = 0;

            link->level = baseLevel;
            break;

        case SB_CHAR_TYPE__BN:
            bnEquivalent = SBTrue;
            break;

        case SB_CHAR_TYPE__NIL:
            forceFinish = SBTrue;
            link->level = baseLevel;
            break;
        }

        /* Rule X9 */
        if (bnEquivalent) {
            /*
             * The type of this link is BN equivalent, so abandon it and
             * continue the loop.
             */
            SBBidiLinkAbandonNext(priorLink);
            continue;
        }

        if (sor == SB_CHAR_TYPE__NIL) {
            sor = SB_LEVEL_TO_TYPE(SB_MAX(baseLevel, link->level));
            firstLink = link;
            priorLevel = link->level;
        } else if (priorLevel != link->level || forceFinish) {
            SBLevelRun levelRun;
            SBLevel currentLevel;

            /*
             * Since the level has changed at this link, therefore the run must
             * end at prior link.
             */
            lastLink = priorLink;

            /*
             * Save the current level i.e. level of the next run.
             */
            currentLevel = link->level;
            /*
             * Now we have both the prior level and the current level i.e.
             * unchanged levels of both the current run and the next run.
             * So, identify eor of the current run.
             * Note:
             *     sor of the run has been already determined at this stage.
             */
            eor = SB_LEVEL_TO_TYPE(SB_MAX(priorLevel, currentLevel));

            SBLevelRunInitialize(&levelRun, firstLink, lastLink, sor, eor);
            _SBProcessRun(support, levelRun, forceFinish);

            /*
             * The sor of next run (if any) should be technically equal to eor
             * of this run.
             */
            sor = eor;
            /*
             * The next run (if any) will start from this index.
             */
            firstLink = link;

            priorLevel = currentLevel;
        }

        priorLink = link;
    };
}

static void _SBProcessRun(_SBParagraphSupportRef support, SBLevelRun levelRun, SBBoolean forceFinish) {
    SBRunQueueRef queue = &support->runQueue;
    SBRunQueueEnqueue(queue, levelRun);

    if (queue->shouldDequeue || forceFinish) {
        SBIsolatingRunRef isolatingRun = &support->isolatingRun;
        SBLevelRunRef peek;

        /* Rule X10 */
        for (; queue->count != 0; SBRunQueueDequeue(queue)) {
            peek = queue->peek;
            if (SB_RUN_KIND__IS_ATTACHED_TERMINATING(peek->kind)) {
                continue;
            }

            isolatingRun->baseLevelRun = peek;
            SBIsolatingRunResolve(isolatingRun);
        }
    }
}

static void _SBSaveLevels(SBBidiChainRef chain, SBLevel *levels, SBLevel baseLevel) {
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

SBParagraphRef SBParagraphCreateWithUnicodeCharacters(SBUnichar *characters, SBUInteger length, SBBaseDirection direction, SBParagraphOptions options) {
    if (characters && length) {
        SBUInteger runCount;
        
        SBParagraphRef paragraph;
        _SBParagraphSupportRef support;
        SBLevel baseLevel;
        
        SB_LOG_BLOCK_OPENER("Input");
        SB_LOG_STATEMENT("Characters", 1, SB_LOG_CHARACTERS_ARRAY(characters, length));
        SB_LOG_STATEMENT("Direction",  1, SB_LOG_BASE_DIRECTION(direction));
        SB_LOG_BLOCK_CLOSER();
        
        paragraph = _SBParagraphAllocate(length);
        runCount = _SBDetermineCharTypes(characters, paragraph->fixedTypes, length);
        
        SB_LOG_BLOCK_OPENER("Determined Types");
        SB_LOG_STATEMENT("Types",  1, SB_LOG_CHAR_TYPES_ARRAY(paragraph->fixedTypes, length));
        SB_LOG_BLOCK_CLOSER();
        
        support = _SBParagraphSupportAllocate(runCount + 1);
        _SBParagraphSupportInitialize(support, characters, paragraph->fixedTypes, paragraph->fixedLevels, length);
        
        baseLevel = _SBDetermineParagraphLevel(&support->bidiChain, direction);
        
        SB_LOG_BLOCK_OPENER("Determined Paragraph Level");
        SB_LOG_STATEMENT("Level", 1, SB_LOG_LEVEL(baseLevel));
        SB_LOG_BLOCK_CLOSER();

        support->isolatingRun.characters = characters;
        support->isolatingRun.paragraphLevel = baseLevel;
        
        _SBDetermineLevels(support, baseLevel);
        _SBSaveLevels(&support->bidiChain, support->refLevels, baseLevel);
        
        SB_LOG_BLOCK_OPENER("Determined Levels");
        SB_LOG_STATEMENT("Levels",  1, SB_LOG_LEVELS_ARRAY(paragraph->fixedLevels, length));
        SB_LOG_BLOCK_CLOSER();
        
        paragraph->length = length;
        paragraph->baseLevel = baseLevel;
        paragraph->_retainCount = 1;
        
        _SBParagraphSupportDeallocate(support);
        
        SB_LOG_BREAKER();
        
        return paragraph;
    }
    
    return NULL;
}

SBLevel SBParagraphGetBaseLevel(SBParagraphRef paragraph) {
    return paragraph->baseLevel;
}

SBParagraphRef SBParagraphRetain(SBParagraphRef paragraph) {
    if (paragraph) {
        ++paragraph->_retainCount;
    }
    
    return paragraph;
}

void SBParagraphRelease(SBParagraphRef paragraph) {
    if (paragraph && --paragraph->_retainCount == 0) {
        free(paragraph);
    }
}
