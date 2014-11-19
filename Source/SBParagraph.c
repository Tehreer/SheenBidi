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
 ((l) & 1)                              \
 ? _SB_CHAR_TYPE__R                     \
 : _SB_CHAR_TYPE__L                     \
)

struct __SBParagraphSupport;
typedef struct __SBParagraphSupport __SBParagraphSupport;
typedef __SBParagraphSupport *__SBParagraphSupportRef;

struct __SBParagraphSupport {
    SBUnichar *refCharacters;
    _SBCharType *refTypes;
    SBLevel *refLevels;
    _SBBidiLink *fixedLinks;
    _SBBidiChain bidiChain;
    _SBStatusStack statusStack;
    _SBRunQueue runQueue;
    _SBIsolatingRun isolatingRun;
};

static __SBParagraphSupportRef __SBParagraphSupportAllocate(SBUInteger linkCount);
static void __SBParagraphSupportInitialize(__SBParagraphSupportRef support, SBUnichar *characters, _SBCharType *types, SBLevel *levels, SBUInteger length);
static void __SBParagraphSupportDeallocate(__SBParagraphSupportRef support);

static SBParagraphRef __SBParagraphAllocate(SBUInteger length);

static SBUInteger __SBDetermineCharTypes(SBUnichar *characters, _SBCharType *types, SBUInteger length);
static void __SBPopulateBidiChain(_SBBidiChainRef chain, _SBBidiLink *links, _SBCharType *types, SBUInteger length);

static _SBBidiLinkRef __SBSkipIsolatingRun(_SBBidiLinkRef skipLink, _SBBidiLinkRef breakLink);
static SBLevel __SBDetermineBaseLevel(_SBBidiLinkRef skipLink, _SBBidiLinkRef breakLink, SBLevel defaultLevel, SBBoolean isIsolate);
static SBLevel __SBDetermineParagraphLevel(_SBBidiChainRef chain, SBBaseDirection baseDirection);

static void __SBDetermineLevels(__SBParagraphSupportRef support, SBLevel baseLevel);
static void __SBProcessRun(__SBParagraphSupportRef support, _SBLevelRun levelRun, SBBoolean forceFinish);
static void __SBSaveLevels(_SBBidiChainRef chain, SBLevel *levels, SBLevel baseLevel);

static __SBParagraphSupportRef __SBParagraphSupportAllocate(SBUInteger linkCount) {
    const SBUInteger sizeSupport = sizeof(__SBParagraphSupport);
    const SBUInteger sizeLinks   = sizeof(_SBBidiLink) * (linkCount + 1);

    const SBUInteger sizeMemory  = sizeSupport
                                 + sizeLinks;

    SBUInt8 *memory = (SBUInt8 *)malloc(sizeMemory);

    SBUInteger offset = 0;
    __SBParagraphSupportRef support = (__SBParagraphSupportRef)(memory + offset);

    offset += sizeSupport;
    support->fixedLinks = (_SBBidiLink *)(memory + offset);

    return support;
}

static void __SBParagraphSupportInitialize(__SBParagraphSupportRef support, SBUnichar *characters, _SBCharType *types, SBLevel *levels, SBUInteger length) {
    support->refCharacters = characters;
    support->refTypes = types;
    support->refLevels = levels;

    _SBBidiChainInitialize(&support->bidiChain);
    _SBStatusStackInitialize(&support->statusStack);
    _SBRunQueueInitialize(&support->runQueue);
    _SBIsolatingRunInitialize(&support->isolatingRun);

    __SBPopulateBidiChain(&support->bidiChain, support->fixedLinks, types, length);
}

static void __SBParagraphSupportDeallocate(__SBParagraphSupportRef support) {
    _SBStatusStackInvalidate(&support->statusStack);
    _SBRunQueueInvalidate(&support->runQueue);
    _SBIsolatingRunInvalidate(&support->isolatingRun);
    free(support);
}

static SBParagraphRef __SBParagraphAllocate(SBUInteger length) {
    const SBUInteger sizeParagraph = sizeof(SBParagraph);
    const SBUInteger sizeTypes     = sizeof(_SBCharType) * length;
    const SBUInteger sizeLevels    = sizeof(SBLevel)     * length;

    const SBUInteger sizeMemory    = sizeParagraph
                                   + sizeTypes
                                   + sizeLevels;

    SBUInt8 *memory = (SBUInt8 *)malloc(sizeMemory);

    SBUInteger offset = 0;
    SBParagraphRef paragraph = (SBParagraphRef)(memory + offset);

    offset += sizeParagraph;
    paragraph->fixedTypes = (_SBCharType *)(memory + offset);

    offset += sizeTypes;
    paragraph->fixedLevels = (SBLevel *)(memory + offset);

    return paragraph;
}

static SBUInteger __SBDetermineCharTypes(SBUnichar *characters, _SBCharType *types, SBUInteger length) {
    SBUInteger linkCount;
    _SBCharType newType;
    SBUInteger index;

    linkCount = 0;
    newType = _SB_CHAR_TYPE__NIL;

    for (index = 0; index < length; ++index) {
        SBUnichar character = characters[index];
        _SBCharType priorType = newType;

        newType = _SBCharTypeDetermineForUnicode(character);

        switch (newType) {
        case _SB_CHAR_TYPE__ON:
        case _SB_CHAR_TYPE__FORMAT_CASE:
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

static void __SBPopulateBidiChain(_SBBidiChainRef chain, _SBBidiLink *links, _SBCharType *types, SBUInteger length) {
    _SBBidiLinkRef priorLink;
    _SBBidiLinkRef newLink;
    _SBCharType type;
    SBUInteger index;

    priorLink = links;
    newLink = links;
    type = _SB_CHAR_TYPE__NIL;

#define _SB_ADD_CONSECUTIVE_LINK(t)                     \
{                                                       \
    priorLink->length = index - priorLink->offset;      \
    priorLink = newLink;                                \
                                                        \
    newLink->offset = index;                            \
    newLink->type = t;                                  \
                                                        \
    _SBBidiChainAddLink(chain, newLink++);              \
}

    for (index = 0; index < length; index++) {
        _SBCharType priorType = type;
        type = types[index];

        switch (type) {
        case _SB_CHAR_TYPE__ON:
        case _SB_CHAR_TYPE__FORMAT_CASE:
            _SB_ADD_CONSECUTIVE_LINK(type);
            break;

        default:
            if (type != priorType) {
                _SB_ADD_CONSECUTIVE_LINK(type);
            }
            break;
        }
    }

    _SB_ADD_CONSECUTIVE_LINK(_SB_CHAR_TYPE__NIL);

#undef _SB_ADD_CONSECUTIVE_LINK
}

static _SBBidiLinkRef __SBSkipIsolatingRun(_SBBidiLinkRef skipLink, _SBBidiLinkRef breakLink) {
    _SBBidiLinkRef link;
    SBUInteger depth = 1;

    for (link = skipLink->next; link != breakLink; link = link->next) {
        _SBCharType type = link->type;

        switch (type) {
        case _SB_CHAR_TYPE__ISOLATE_INITIATOR_CASE:
            ++depth;
            break;

        case _SB_CHAR_TYPE__PDI:
            if (--depth == 0) {
                return link;
            }
            break;
        }
    }

    return NULL;
}

static SBLevel __SBDetermineBaseLevel(_SBBidiLinkRef skipLink, _SBBidiLinkRef breakLink, SBLevel defaultLevel, SBBoolean isIsolate) {
    _SBBidiLinkRef link;

    /* Rules P2, P3 */
    for (link = skipLink->next; link != breakLink; link = link->next) {
        _SBCharType type = link->type;

        switch (type) {
        case _SB_CHAR_TYPE__L:
            return 0;

        case _SB_CHAR_TYPE__AL:
        case _SB_CHAR_TYPE__R:
            return 1;

        case _SB_CHAR_TYPE__ISOLATE_INITIATOR_CASE:
            link = __SBSkipIsolatingRun(link, breakLink);
            if (!link) {
                goto Default;
            }
            break;

        case _SB_CHAR_TYPE__PDI:
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

static SBLevel __SBDetermineParagraphLevel(_SBBidiChainRef chain, SBBaseDirection baseDirection) {
    SBLevel level;

    switch (baseDirection) {
    case SBBaseDirectionLTR:
        level = 0;
        break;

    case SBBaseDirectionRTL:
        level = 1;
        break;

    default:
        level = __SBDetermineBaseLevel(chain->rollerLink, chain->rollerLink,
                                       baseDirection == SBBaseDirectionAutoLTR ? 0 : 1,
                                       SBFalse);
        break;
    }

    return level;
}

static void __SBDetermineLevels(__SBParagraphSupportRef support, SBLevel baseLevel) {
    _SBBidiChainRef chain = &support->bidiChain;
    _SBStatusStackRef stack = &support->statusStack;
    _SBBidiLinkRef roller = chain->rollerLink;
    _SBBidiLinkRef link;

    _SBBidiLinkRef priorLink;
    _SBBidiLinkRef firstLink;
    _SBBidiLinkRef lastLink;

    SBLevel priorLevel;
    _SBCharType sor;
    _SBCharType eor;

    SBUInteger overIsolate;
    SBUInteger overEmbedding;
    SBUInteger validIsolate;

    priorLink = roller;
    firstLink = NULL;
    lastLink = NULL;

    priorLevel = baseLevel;
    sor = _SB_CHAR_TYPE__NIL;

    /* Rule X1 */
    overIsolate = 0;
    overEmbedding = 0;
    validIsolate = 0;

    _SBStatusStackPush(stack, baseLevel, _SB_CHAR_TYPE__ON, SBFalse);

    for (link = roller->next; link != roller; link = link->next) {
        SBBoolean forceFinish;
        SBBoolean bnEquivalent;

        _SBCharType type;

        forceFinish = SBFalse;
        bnEquivalent = SBFalse;

        type = link->type;

#define SB_LEAST_GREATER_ODD_LEVEL()                                        \
(                                                                           \
        (_SBStatusStackGetEmbeddingLevel(stack) + 1) | 1                    \
)

#define SB_LEAST_GREATER_EVEN_LEVEL()                                       \
(                                                                           \
        (_SBStatusStackGetEmbeddingLevel(stack) + 2) & ~1                   \
)

#define SB_PUSH_EMBEDDING(l, o)                                             \
{                                                                           \
        SBLevel newLevel;                                                   \
                                                                            \
        bnEquivalent = SBTrue;                                              \
        newLevel = l;                                                       \
                                                                            \
        if (newLevel <= SB_LEVEL_MAX && !overIsolate && !overEmbedding) {   \
            _SBStatusStackPush(stack, newLevel, o, SBFalse);                \
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
        link->level = _SBStatusStackGetEmbeddingLevel(stack);               \
        newLevel = l;                                                       \
                                                                            \
        if (newLevel <= SB_LEVEL_MAX && !overIsolate && !overEmbedding) {   \
            ++validIsolate;                                                 \
            _SBStatusStackPush(stack, newLevel, o, SBTrue);                 \
        } else {                                                            \
            ++overIsolate;                                                  \
        }                                                                   \
}

        switch (type) {
        /* Rule X2 */
        case _SB_CHAR_TYPE__RLE:
            SB_PUSH_EMBEDDING(SB_LEAST_GREATER_ODD_LEVEL(), _SB_CHAR_TYPE__ON);
            break;

        /* Rule X3 */
        case _SB_CHAR_TYPE__LRE:
            SB_PUSH_EMBEDDING(SB_LEAST_GREATER_EVEN_LEVEL(), _SB_CHAR_TYPE__ON);
            break;

        /* Rule X4 */
        case _SB_CHAR_TYPE__RLO:
            SB_PUSH_EMBEDDING(SB_LEAST_GREATER_ODD_LEVEL(), _SB_CHAR_TYPE__R);
            break;

        /* Rule X5 */
        case _SB_CHAR_TYPE__LRO:
            SB_PUSH_EMBEDDING(SB_LEAST_GREATER_EVEN_LEVEL(), _SB_CHAR_TYPE__L);
            break;

        /* Rule X5a */
        case _SB_CHAR_TYPE__RLI:
            SB_PUSH_ISOLATE(SB_LEAST_GREATER_ODD_LEVEL(), _SB_CHAR_TYPE__ON);
            break;

        /* Rule X5b */
        case _SB_CHAR_TYPE__LRI:
            SB_PUSH_ISOLATE(SB_LEAST_GREATER_EVEN_LEVEL(), _SB_CHAR_TYPE__ON);
            break;

        /* Rule X5c */
        case _SB_CHAR_TYPE__FSI:
        {
            SBBoolean isRTL = (__SBDetermineBaseLevel(link, roller, 0, SBTrue) == 1);
            SB_PUSH_ISOLATE(isRTL
                             ? SB_LEAST_GREATER_ODD_LEVEL()
                             : SB_LEAST_GREATER_EVEN_LEVEL(),
                            _SB_CHAR_TYPE__ON);
            break;
        }

        /* Rule X6 */
        default:
            link->level = _SBStatusStackGetEmbeddingLevel(stack);

            if (_SBStatusStackGetOverrideStatus(stack) != _SB_CHAR_TYPE__ON) {
                link->type = _SBStatusStackGetOverrideStatus(stack);

                if (priorLink->type == link->type
                    && priorLink->level == link->level) {
                    /*
                     * Properties of this link are same as previous link,
                     * therefore merge it and continue the loop.
                     */
                    _SBBidiLinkMergeNext(priorLink);
                    continue;
                }
            }
            break;

        /* Rule X6a */
        case _SB_CHAR_TYPE__PDI:
            if (overIsolate != 0) {
                --overIsolate;
            } else if (validIsolate == 0) {
                /* Do nothing */
            } else {
                overEmbedding = 0;

                while (!_SBStatusStackGetIsolateStatus(stack)) {
                    _SBStatusStackPop(stack);
                };
                _SBStatusStackPop(stack);

                --validIsolate;
            }

            link->level = _SBStatusStackGetEmbeddingLevel(stack);
            break;

        /* Rule X7 */
        case _SB_CHAR_TYPE__PDF:
            bnEquivalent = SBTrue;

            if (overIsolate != 0) {
                /* do nothing */
            } else if (overEmbedding != 0) {
                --overEmbedding;
            } else if (!_SBStatusStackGetIsolateStatus(stack) && stack->count >= 2) {
                _SBStatusStackPop(stack);
            }
            break;

        /* Rule X8 */
        case _SB_CHAR_TYPE__B:
            /* These values are reset for clarity, in this implementation B
             * can only occur as the last code in the array.
             */
            _SBStatusStackSetEmpty(stack);

            overIsolate = 0;
            overEmbedding = 0;
            validIsolate = 0;

            link->level = baseLevel;
            break;

        case _SB_CHAR_TYPE__BN:
            bnEquivalent = SBTrue;
            break;

        case _SB_CHAR_TYPE__NIL:
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
            _SBBidiLinkAbandonNext(priorLink);
            continue;
        }

        if (sor == _SB_CHAR_TYPE__NIL) {
            sor = SB_LEVEL_TO_TYPE(SB_MAX(baseLevel, link->level));
            firstLink = link;
            priorLevel = link->level;
        } else if (priorLevel != link->level || forceFinish) {
            _SBLevelRun levelRun;
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

            _SBLevelRunInitialize(&levelRun, firstLink, lastLink, sor, eor);
            __SBProcessRun(support, levelRun, forceFinish);

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

static void __SBProcessRun(__SBParagraphSupportRef support, _SBLevelRun levelRun, SBBoolean forceFinish) {
    _SBRunQueueRef queue = &support->runQueue;
    _SBRunQueueEnqueue(queue, levelRun);

    if (queue->shouldDequeue || forceFinish) {
        _SBIsolatingRunRef isolatingRun = &support->isolatingRun;
        _SBLevelRunRef peek;

        /* Rule X10 */
        for (; queue->count != 0; _SBRunQueueDequeue(queue)) {
            peek = queue->peek;
            if (_SB_RUN_KIND__IS_ATTACHED_TERMINATING(peek->kind)) {
                continue;
            }

            isolatingRun->baseLevelRun = peek;
            _SBIsolatingRunResolve(isolatingRun);
        }
    }
}

static void __SBSaveLevels(_SBBidiChainRef chain, SBLevel *levels, SBLevel baseLevel) {
    _SBBidiLinkRef roller = chain->rollerLink;
    _SBBidiLinkRef link;

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
        __SBParagraphSupportRef support;
        SBLevel baseLevel;
        
        _SB_LOG_BLOCK_OPENER("Input");
        _SB_LOG_STATEMENT("Characters", 1, _SB_LOG_CHARACTERS_ARRAY(characters, length));
        _SB_LOG_STATEMENT("Direction",  1, _SB_LOG_BASE_DIRECTION(direction));
        _SB_LOG_BLOCK_CLOSER();
        
        paragraph = __SBParagraphAllocate(length);
        runCount = __SBDetermineCharTypes(characters, paragraph->fixedTypes, length);
        
        _SB_LOG_BLOCK_OPENER("Determined Types");
        _SB_LOG_STATEMENT("Types",  1, _SB_LOG_CHAR_TYPES_ARRAY(paragraph->fixedTypes, length));
        _SB_LOG_BLOCK_CLOSER();
        
        support = __SBParagraphSupportAllocate(runCount + 1);
        __SBParagraphSupportInitialize(support, characters, paragraph->fixedTypes, paragraph->fixedLevels, length);
        
        baseLevel = __SBDetermineParagraphLevel(&support->bidiChain, direction);
        
        _SB_LOG_BLOCK_OPENER("Determined Paragraph Level");
        _SB_LOG_STATEMENT("Level", 1, _SB_LOG_LEVEL(baseLevel));
        _SB_LOG_BLOCK_CLOSER();

        support->isolatingRun.characters = characters;
        support->isolatingRun.paragraphLevel = baseLevel;
        
        __SBDetermineLevels(support, baseLevel);
        __SBSaveLevels(&support->bidiChain, support->refLevels, baseLevel);
        
        _SB_LOG_BLOCK_OPENER("Determined Levels");
        _SB_LOG_STATEMENT("Levels",  1, _SB_LOG_LEVELS_ARRAY(paragraph->fixedLevels, length));
        _SB_LOG_BLOCK_CLOSER();
        
        paragraph->length = length;
        paragraph->baseLevel = baseLevel;
        paragraph->_retainCount = 1;
        
        __SBParagraphSupportDeallocate(support);
        
        _SB_LOG_BREAKER();
        
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
