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

#include "SBCharType.h"
#include "SBCharTypeLookup.h"

#include "SBBracketType.h"
#include "SBPairingLookup.h"

#include "SBStatusStack.h"
#include "SBRunQueue.h"
#include "SBBracketQueue.h"

#include "SBRunChain.h"
#include "SBLevelRun.h"
#include "SBIsolatingRun.h"

#include "SBLog.h"
#include "SBParagraph.h"

#define SB_MAX(a, b)                    \
(                                       \
 (a) > (b) ? (a) : (b)                  \
)

#define SB_LEVEL_MAX                    125

#define SB_LEVEL_TO_EXACT_TYPE(l)       \
(                                       \
 ((l) & 1)                              \
 ? _SB_CHAR_TYPE__R                     \
 : _SB_CHAR_TYPE__L                     \
)

#define SB_LEVEL_TO_OPPOSITE_TYPE(l)    \
(                                       \
 ((l) & 1)                              \
 ? _SB_CHAR_TYPE__L                     \
 : _SB_CHAR_TYPE__R                     \
)

struct __SBParagraphSupport;
typedef struct __SBParagraphSupport __SBParagraphSupport;
typedef __SBParagraphSupport *__SBParagraphSupportRef;

struct __SBParagraphSupport {
    SBUnichar *refCharacters;
    _SBCharType *refTypes;
    SBLevel *refLevels;
    _SBRunLink *fixedLinks;
    _SBRunChain runChain;
    _SBStatusStack statusStack;
    _SBRunQueue runQueue;
    _SBBracketQueue bracketQueue;
    _SBIsolatingRun isolatingRun;
};

static __SBParagraphSupportRef __SBParagraphSupportAllocate(SBUInteger linkCount);
static void __SBParagraphSupportInitialize(__SBParagraphSupportRef support, SBUnichar *characters, _SBCharType *types, SBLevel *levels, SBUInteger length);
static void __SBParagraphSupportDeallocate(__SBParagraphSupportRef support);

static SBParagraphRef __SBParagraphAllocate(SBUInteger length);

static SBUInteger __SBDetermineCharTypes(SBUnichar *characters, _SBCharType *types, SBUInteger length);
static void __SBPopulateRunChain(_SBRunChainRef chain, _SBRunLink *links, _SBCharType *types, SBUInteger length);

static SBBoolean __SBSkipIsolatingRun(_SBRunLinkIterRef iter);
static SBLevel __SBDetermineBaseLevel(_SBRunLinkIter iter, SBLevel defaultLevel, SBBoolean isIsolate);
static SBLevel __SBDetermineParagraphLevel(_SBRunChainRef chain, SBBaseDirection baseDirection);

static void __SBDetermineLevels(__SBParagraphSupportRef support, SBLevel baseLevel);
static void __SBProcessRun(__SBParagraphSupportRef support, _SBLevelRun levelRun, SBLevel baseLevel, SBBoolean forceFinish);
static void __SBSaveLevels(_SBRunChainRef chain, SBLevel *levels, SBLevel baseLevel);

static _SBRunLinkRef __SBResolveWeakTypes(_SBIsolatingRunRef isolatingRun, _SBRunLinkRef dummyLink);
static void __SBResolveBrackets(_SBIsolatingRunRef isolatingRun, _SBBracketQueueRef queue, SBUnichar *characters);
static void __SBResolveAvailableBracketPairs(_SBIsolatingRunRef isolatingRun, _SBBracketQueueRef queue);
static void __SBResolveNeutrals(_SBIsolatingRunRef isolatingRun);
static void __SBResolveImplicitLevels(_SBIsolatingRunRef isolatingRun);

static __SBParagraphSupportRef __SBParagraphSupportAllocate(SBUInteger linkCount) {
    const SBUInteger sizeSupport = sizeof(__SBParagraphSupport);
    const SBUInteger sizeLinks   = sizeof(_SBRunLink) * (linkCount + 1);

    const SBUInteger sizeMemory  = sizeSupport
                                 + sizeLinks;

    SBUInt8 *memory = (SBUInt8 *)malloc(sizeMemory);

    SBUInteger offset = 0;
    __SBParagraphSupportRef support = (__SBParagraphSupportRef)(memory + offset);

    offset += sizeSupport;
    support->fixedLinks = (_SBRunLink *)(memory + offset);

    return support;
}

static void __SBParagraphSupportInitialize(__SBParagraphSupportRef support, SBUnichar *characters, _SBCharType *types, SBLevel *levels, SBUInteger length) {
    support->refCharacters = characters;
    support->refTypes = types;
    support->refLevels = levels;

    _SBRunChainInitialize(&support->runChain);
    _SBStatusStackInitialize(&support->statusStack);
    _SBRunQueueInitialize(&support->runQueue);
    _SBBracketQueueInitialize(&support->bracketQueue);
    _SBIsolatingRunInitialize(&support->isolatingRun, &support->runChain);

    __SBPopulateRunChain(&support->runChain, support->fixedLinks, types, length);
}

static void __SBParagraphSupportDeallocate(__SBParagraphSupportRef support) {
    _SBStatusStackInvalidate(&support->statusStack);
    _SBRunQueueInvalidate(&support->runQueue);
    _SBBracketQueueInvalidate(&support->bracketQueue);
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

static void __SBPopulateRunChain(_SBRunChainRef chain, _SBRunLink *links, _SBCharType *types, SBUInteger length) {
    _SBRunLinkRef priorLink;
    _SBRunLinkRef newLink;
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
    _SBRunChainAddLink(chain, newLink++);               \
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

static SBBoolean __SBSkipIsolatingRun(_SBRunLinkIterRef iter) {
    SBUInteger depth = 1;

    while (_SBRunLinkIterMoveNext(iter)) {
        _SBCharType type = iter->current->type;

        switch (type) {
        case _SB_CHAR_TYPE__ISOLATE_INITIATOR_CASE:
            ++depth;
            break;

        case _SB_CHAR_TYPE__PDI:
            if (--depth == 0) {
                return SBTrue;
            }
            break;
        }
    }

    return SBFalse;
}

static SBLevel __SBDetermineBaseLevel(_SBRunLinkIter iter, SBLevel defaultLevel, SBBoolean isIsolate) {
    /* Rules P2, P3 */
    while (_SBRunLinkIterMoveNext(&iter)) {
        _SBCharType type = iter.current->type;

        switch (type) {
        case _SB_CHAR_TYPE__L:
            return 0;

        case _SB_CHAR_TYPE__AL:
        case _SB_CHAR_TYPE__R:
            return 1;

        case _SB_CHAR_TYPE__ISOLATE_INITIATOR_CASE:
            if (!__SBSkipIsolatingRun(&iter)) {
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

static SBLevel __SBDetermineParagraphLevel(_SBRunChainRef chain, SBBaseDirection baseDirection) {
    SBLevel level;

    switch (baseDirection) {
    case SBBaseDirectionLTR:
        level = 0;
        break;

    case SBBaseDirectionRTL:
        level = 1;
        break;

    default:
        level = __SBDetermineBaseLevel(_SBRunChainGetIter(chain),
                                       baseDirection == SBBaseDirectionAutoLTR ? 0 : 1,
                                       SBFalse);
        break;
    }

    return level;
}

static void __SBDetermineLevels(__SBParagraphSupportRef support, SBLevel baseLevel) {
    _SBRunChainRef chain = &support->runChain;
    _SBStatusStackRef stack = &support->statusStack;
    _SBRunLinkIter iter;

    _SBRunLinkRef firstLink;
    _SBRunLinkRef lastLink;
    _SBRunLinkRef priorLink;

    SBLevel priorLevel;
    _SBCharType sor;
    _SBCharType eor;

    SBUInteger overIsolate;
    SBUInteger overEmbedding;
    SBUInteger validIsolate;

    firstLink = NULL;
    lastLink = NULL;
    priorLink = chain->rollerLink;

    priorLevel = baseLevel;
    sor = _SB_CHAR_TYPE__NIL;

    /* Rule X1 */
    overIsolate = 0;
    overEmbedding = 0;
    validIsolate = 0;

    _SBStatusStackPush(stack, baseLevel, _SB_CHAR_TYPE__ON, SBFalse);

    iter = _SBRunChainGetIter(chain);
    while (_SBRunLinkIterMoveNext(&iter)) {
        SBBoolean forceFinish;
        SBBoolean bnEquivalent;

        _SBRunLinkRef link;
        _SBCharType type;

        forceFinish = SBFalse;
        bnEquivalent = SBFalse;

        link = iter.current;
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
            SBBoolean isRTL = (__SBDetermineBaseLevel(iter, 0, SBTrue) == 1);
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
                    _SBRunLinkMergeNext(priorLink);
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
            _SBRunLinkAbandonNext(priorLink);
            continue;
        }

        if (sor == _SB_CHAR_TYPE__NIL) {
            sor = SB_LEVEL_TO_EXACT_TYPE(SB_MAX(baseLevel, link->level));
            firstLink = link;
            priorLevel = link->level;
        } else if (priorLevel != link->level || forceFinish) {
            _SBLevelRun levelRun;
            SBLevel currentLevel;

            /*
             * Since the level has changed at this index, therefore, the run
             * must end at the last index.
             */
            lastLink = priorLink;

            /*
             * Save the current level i.e. level of the next run.
             */
            currentLevel = link->level;
            /*
             * Now we have both the last level and the current level i.e.
             * unchanged levels of both the current run and the next run.
             * So, identify the eor of the current run.
             * Note:
             *     sor of the run has been already determined at this stage.
             */
            eor = SB_LEVEL_TO_EXACT_TYPE(SB_MAX(priorLevel, currentLevel));

            _SBLevelRunInitialize(&levelRun, firstLink, lastLink, sor, eor);
            __SBProcessRun(support, levelRun, baseLevel, forceFinish);

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

static void __SBProcessRun(__SBParagraphSupportRef support, _SBLevelRun levelRun, SBLevel baseLevel, SBBoolean forceFinish) {
    _SBRunQueueRef queue = &support->runQueue;
    _SBRunQueueEnqueue(queue, levelRun);

    if (queue->shouldDequeue || forceFinish) {
        _SBIsolatingRunRef isolatingRun = &support->isolatingRun;
        _SBLevelRunRef peek;
        _SBRunLinkRef subsequent;
        _SBRunLinkRef last;

        /* Rule X10 */
        for (; queue->count != 0; _SBRunQueueDequeue(queue)) {
            peek = queue->peek;
            if (_SB_RUN_KIND__IS_ATTACHED_TERMINATING(peek->kind)) {
                continue;
            }

            _SBIsolatingRunBuild(isolatingRun, peek, baseLevel);
            subsequent = isolatingRun->lastRun->subsequentLink;

            _SB_LOG_BLOCK_OPENER("Identified Isolating Run");
            _SB_LOG_STATEMENT("Range", 1, _SB_LOG_ISOLATING_RUN_RANGE(isolatingRun));
            _SB_LOG_STATEMENT("Types", 1, _SB_LOG_LINK_TYPES(_SBIsolatingRunGetIter(isolatingRun)));
            _SB_LOG_STATEMENT("Level", 1, _SB_LOG_LEVEL(_SBIsolatingRunGetLevel(isolatingRun)));
            _SB_LOG_STATEMENT("SOS", 1, _SB_LOG_CHAR_TYPE(isolatingRun->sos));
            _SB_LOG_STATEMENT("EOS", 1, _SB_LOG_CHAR_TYPE(isolatingRun->eos));

            /* Rules W1-W7 */
            last = __SBResolveWeakTypes(isolatingRun, support->runChain.rollerLink);
            _SB_LOG_BLOCK_OPENER("Resolved Weak Types");
            _SB_LOG_STATEMENT("Types", 1, _SB_LOG_LINK_TYPES(_SBIsolatingRunGetIter(isolatingRun)));
            _SB_LOG_BLOCK_CLOSER();

            /* Rule N0 */
            __SBResolveBrackets(isolatingRun, &support->bracketQueue, support->refCharacters);
            _SB_LOG_BLOCK_OPENER("Resolved Brackets");
            _SB_LOG_STATEMENT("Types", 1, _SB_LOG_LINK_TYPES(_SBIsolatingRunGetIter(isolatingRun)));
            _SB_LOG_BLOCK_CLOSER();

            /* Rules N1, N2 */
            __SBResolveNeutrals(isolatingRun);
            _SB_LOG_BLOCK_OPENER("Resolved Neutrals");
            _SB_LOG_STATEMENT("Types", 1, _SB_LOG_LINK_TYPES(_SBIsolatingRunGetIter(isolatingRun)));
            _SB_LOG_BLOCK_CLOSER();

            /* Rules I1, I2 */
            __SBResolveImplicitLevels(isolatingRun);
            _SB_LOG_BLOCK_OPENER("Resolved Implicit Levels");
            _SB_LOG_STATEMENT("Levels", 1, _SB_LOG_LINK_LEVELS(_SBIsolatingRunGetIter(isolatingRun)));
            _SB_LOG_BLOCK_CLOSER();
            _SB_LOG_BLOCK_CLOSER();

            /* Invalidate the isolating run. */
            _SBIsolatingRunInvalidate(isolatingRun);
            /*
             * Replace the new final link (of isolating run) with last
             * subsequent link.
             */
            _SBRunLinkReplaceNext(last, subsequent);
        }
    }
}

static void __SBSaveLevels(_SBRunChainRef chain, SBLevel *levels, SBLevel baseLevel) {
    _SBRunLinkIter iter;
    SBUInteger index;
    SBLevel level;

    index = 0;
    level = baseLevel;

    iter = _SBRunChainGetIter(chain);
    while (_SBRunLinkIterMoveNext(&iter)) {
        _SBRunLinkRef link = iter.current;
        SBUInteger offset = link->offset;

        for (; index < offset; index++) {
            levels[index] = level;
        }

        level = link->level;
    }
}

static _SBRunLinkRef __SBResolveWeakTypes(_SBIsolatingRunRef isolatingRun, _SBRunLinkRef dummyLink) {
    _SBRunLinkIter iter;
    _SBCharType sos;
    _SBRunLinkRef priorLink;

    _SBCharType w1PriorType;
    _SBCharType w2StrongType;
    _SBCharType w4PriorType;
    _SBCharType w5PriorType;
    _SBCharType w7StrongType;

    /*
     * The dummy link must be empty.
     */
    SBAssert(dummyLink->type == _SB_CHAR_TYPE__NIL);

    sos = isolatingRun->sos;
    priorLink = dummyLink;

    w1PriorType = sos;
    w2StrongType = sos;

    iter = _SBIsolatingRunGetIter(isolatingRun);
    while (_SBRunLinkIterMoveNext(&iter)) {
        _SBRunLinkRef link = iter.current;
        _SBCharType type = link->type;

        /* Rule W1 */
        if (type == _SB_CHAR_TYPE__NSM) {
            /*
             * Change the 'type' variable as well because it can be EN on which
             * W2 depends.
             */
            link->type = type = (_SB_CHAR_TYPE__IS_ISOLATE(w1PriorType)
                                 ? _SB_CHAR_TYPE__ON
                                 : w1PriorType
                                );
        }
        w1PriorType = type;

        /* Rule W2 */
        if (type == _SB_CHAR_TYPE__EN) {
            if (w2StrongType == _SB_CHAR_TYPE__AL) {
                link->type = _SB_CHAR_TYPE__AN;
            }
        }
        /*
         * Rule W3
         * Note: It is safe to apply W3 in 'else-if' statement because it only
         *       depends on type AL. Even if W2 changes EN to AN, there won't
         *       be any harm.
         */
        else if (type == _SB_CHAR_TYPE__AL) {
            link->type = _SB_CHAR_TYPE__R;
        }

        if (_SB_CHAR_TYPE__IS_STRONG(type)) {
            /*
             * Save the strong type as it is checked in W2.
             */
            w2StrongType = type;
        }

        if (type != _SB_CHAR_TYPE__ON && priorLink->type == type) {
            _SBRunLinkMergeNext(priorLink);
        } else {
            priorLink = link;
        }
    }

    priorLink = dummyLink;
    w4PriorType = sos;
    w5PriorType = sos;
    w7StrongType = sos;

    _SBRunLinkIterReset(&iter);
    while (_SBRunLinkIterMoveNext(&iter)) {
        _SBRunLinkRef link = iter.current;
        _SBCharType type = link->type;
        _SBCharType nextType = link->next->type;

        /* Rule W4 */
        if (link->length == 1
            && (type == _SB_CHAR_TYPE__ES || type == _SB_CHAR_TYPE__CS)
            && _SB_CHAR_TYPE__IS_NUMBER(w4PriorType)
            && (w4PriorType == nextType)
            && (w4PriorType == _SB_CHAR_TYPE__EN || type == _SB_CHAR_TYPE__CS))
        {
            /*
             * Change the current type as well because it can be EN on which W5
             * depends.
             */
            link->type = type = w4PriorType;
        }
        w4PriorType = type;

        /* Rule W5 */
        if (type == _SB_CHAR_TYPE__ET
            && (w5PriorType == _SB_CHAR_TYPE__EN || nextType == _SB_CHAR_TYPE__EN))
        {
            /*
             * Change the current type as well because it is EN on which W7
             * depends.
             */
            link->type = type = _SB_CHAR_TYPE__EN;
        }
        w5PriorType = type;

        switch (type) {
            /* Rule W6 */
        case _SB_CHAR_TYPE__ET:
        case _SB_CHAR_TYPE__CS:
        case _SB_CHAR_TYPE__ES:
            link->type = _SB_CHAR_TYPE__ON;
            break;

            /*
             * Rule W7
             * Note: W7 is expected to be applied after W6. However this is not the
             *       case here. The reason is that W6 can only create the type ON
             *       which is not tested in W7 by any means. So it won't affect the
             *       algorithm.
             */
        case _SB_CHAR_TYPE__EN:
            if (w7StrongType == _SB_CHAR_TYPE__L) {
                link->type = _SB_CHAR_TYPE__L;
            }
            break;

            /*
             * Save the strong type for W7.
             * Note: The strong type is expected to be saved after applying W7
             *       because W7 itself creates a strong type. However the strong type
             *       being saved here is based on the type after W5.
             *       This won't effect the algorithm because a single link contains
             *       all consecutive EN types. This means that even if W7 creates a
             *       strong type, it will be saved in next iteration.
             */
        case _SB_CHAR_TYPE__L:
        case _SB_CHAR_TYPE__R:
            w7StrongType = type;
            break;
        }

        if (type != _SB_CHAR_TYPE__ON && priorLink->type == type) {
            _SBRunLinkMergeNext(priorLink);
        } else {
            priorLink = link;
        }
    }

    return priorLink;
}

static void __SBResolveBrackets(_SBIsolatingRunRef isolatingRun, _SBBracketQueueRef queue, SBUnichar *characters) {
    _SBRunLinkIter iter;
    _SBRunLinkRef priorStrongLink;
    SBLevel runLevel;

    priorStrongLink = NULL;
    runLevel = _SBIsolatingRunGetLevel(isolatingRun);

    _SBBracketQueueReset(queue, SB_LEVEL_TO_EXACT_TYPE(runLevel));

    iter = _SBIsolatingRunGetIter(isolatingRun);
    while (_SBRunLinkIterMoveNext(&iter)) {
        _SBRunLinkRef link;
        SBUnichar ch;
        _SBCharType type;

        SBUnichar bracketValue;
        _SBBracketType bracketType;

        link = iter.current;
        type = link->type;

        switch (type) {
        case _SB_CHAR_TYPE__ON:
            ch = characters[link->offset];
            bracketValue = _SBPairingDetermineBracketPair(ch, &bracketType);

            switch (bracketType) {
            case _SB_BRACKET_TYPE__OPEN:
                _SBBracketQueueEnqueue(queue, priorStrongLink, link, ch);
                break;

            case _SB_BRACKET_TYPE__CLOSE:
                if (queue->count != 0) {
                    _SBBracketQueueClosePair(queue, link, bracketValue);

                    if (_SBBracketQueueShouldDequeue(queue)) {
                        __SBResolveAvailableBracketPairs(isolatingRun, queue);
                    }
                }
                break;
            }
            break;

        case _SB_CHAR_TYPE__NUMBER_CASE:
            type = _SB_CHAR_TYPE__R;

        case _SB_CHAR_TYPE__R:
        case _SB_CHAR_TYPE__L:
            if (queue->count != 0) {
                _SBBracketQueueSetStrongType(queue, type);
            }

            priorStrongLink = link;
            break;
        }
    }

    __SBResolveAvailableBracketPairs(isolatingRun, queue);
}

static void __SBResolveAvailableBracketPairs(_SBIsolatingRunRef isolatingRun, _SBBracketQueueRef queue) {
    SBLevel runLevel;
    _SBCharType embeddingDirection;
    _SBCharType oppositeDirection;

    runLevel = _SBIsolatingRunGetLevel(isolatingRun);
    embeddingDirection = SB_LEVEL_TO_EXACT_TYPE(runLevel);
    oppositeDirection = SB_LEVEL_TO_OPPOSITE_TYPE(runLevel);

    while (queue->count != 0) {
        _SBRunLinkRef openingLink = _SBBracketQueueGetOpeningLink(queue);
        _SBRunLinkRef closingLink = _SBBracketQueueGetClosingLink(queue);

        if (openingLink && closingLink) {
            _SBCharType innerStrongType;
            _SBCharType pairType;

            innerStrongType = _SBBracketQueueGetStrongType(queue);

            /* Rule: N0.b */
            if (innerStrongType == embeddingDirection) {
                pairType = innerStrongType;
            }
            /* Rule: N0.c */
            else if (innerStrongType == oppositeDirection) {
                _SBRunLinkRef priorStrongLink;
                _SBCharType priorStrongType;

                priorStrongLink = _SBBracketQueueGetPriorStrongLink(queue);

                if (priorStrongLink) {
                    _SBRunLinkRef link;

                    priorStrongType = priorStrongLink->type;
                    if (_SB_CHAR_TYPE__IS_NUMBER(priorStrongType)) {
                        priorStrongType = _SB_CHAR_TYPE__R;
                    }

                    link = priorStrongLink->next;

                    while (link != openingLink) {
                        _SBCharType type = link->type;
                        if (type == _SB_CHAR_TYPE__L || type == _SB_CHAR_TYPE__R) {
                            priorStrongType = type;
                        }

                        link = link->next;
                    }
                } else {
                    priorStrongType = isolatingRun->sos;
                }

                /* Rule: N0.c.1 */
                if (priorStrongType == oppositeDirection) {
                    pairType = oppositeDirection;
                }
                /* Rule: N0.c.2 */
                else {
                    pairType = embeddingDirection;
                }
            }
            /* Rule: N0.d */
            else {
                pairType = _SB_CHAR_TYPE__NIL;
            }

            if (pairType != _SB_CHAR_TYPE__NIL) {
                /* Do the substitution */
                openingLink->type = pairType;
                closingLink->type = pairType;
            }
        }

        _SBBracketQueueDequeue(queue);
    }
}

static void __SBResolveNeutrals(_SBIsolatingRunRef isolatingRun) {
    _SBRunLinkIter iter;
    SBLevel runLevel;
    _SBCharType strongType;
    _SBRunLinkRef neutralLink;

    runLevel = _SBIsolatingRunGetLevel(isolatingRun);
    strongType = isolatingRun->sos;
    neutralLink = NULL;

    iter = _SBIsolatingRunGetIter(isolatingRun);
    while (_SBRunLinkIterMoveNext(&iter)) {
        _SBRunLinkRef link = iter.current;
        _SBCharType type = link->type;
        _SBCharType nextType;

        SBAssert(_SB_CHAR_TYPE__IS_STRONG_OR_NUMBER(type) || _SB_CHAR_TYPE__IS_NEUTRAL_OR_ISOLATE(type));

        switch (type) {
        case _SB_CHAR_TYPE__L:
            strongType = _SB_CHAR_TYPE__L;
            break;

        case _SB_CHAR_TYPE__R:
        case _SB_CHAR_TYPE__NUMBER_CASE:
            strongType = _SB_CHAR_TYPE__R;
            break;

        case _SB_CHAR_TYPE__NEUTRAL_CASE:
        case _SB_CHAR_TYPE__ISOLATE_CASE:
            if (!neutralLink) {
                neutralLink = link;
            }

            nextType = link->next->type;
            if (_SB_CHAR_TYPE__IS_NUMBER(nextType)) {
                nextType = _SB_CHAR_TYPE__R;
            } else if (nextType == _SB_CHAR_TYPE__NIL) {
                nextType = isolatingRun->eos;
            }

            if (_SB_CHAR_TYPE__IS_STRONG(nextType)) {
                /* Rules N1, N2 */
                _SBCharType resolvedType = (strongType == nextType
                                            ? strongType
                                            : SB_LEVEL_TO_EXACT_TYPE(runLevel)
                                           );
                
                do {
                    neutralLink->type = resolvedType;
                    neutralLink = neutralLink->next;
                } while (neutralLink != link->next);
                
                neutralLink = NULL;
            }
            break;
        }
    }
}

static void __SBResolveImplicitLevels(_SBIsolatingRunRef isolatingRun) {
    SBLevel runLevel = _SBIsolatingRunGetLevel(isolatingRun);
    _SBRunLinkIter iter = _SBIsolatingRunGetIter(isolatingRun);
    
    if ((runLevel & 1) == 0) {
        while (_SBRunLinkIterMoveNext(&iter)) {
            _SBRunLinkRef link = iter.current;
            _SBCharType type = link->type;
            
            SBAssert(_SB_CHAR_TYPE__IS_STRONG_OR_NUMBER(type));
            
            /* Rule I1 */
            if (type == _SB_CHAR_TYPE__R) {
                link->level += 1;
            } else if (type != _SB_CHAR_TYPE__L) {
                link->level += 2;
            }
        }
    } else {
        while (_SBRunLinkIterMoveNext(&iter)) {
            _SBRunLinkRef link = iter.current;
            _SBCharType type = link->type;
            
            assert(_SB_CHAR_TYPE__IS_STRONG_OR_NUMBER(type));
            
            /* Rule I2 */
            if (type != _SB_CHAR_TYPE__R) {
                link->level += 1;
            }
        }
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
        
        baseLevel = __SBDetermineParagraphLevel(&support->runChain, direction);
        
        _SB_LOG_BLOCK_OPENER("Determined Paragraph Level");
        _SB_LOG_STATEMENT("Level", 1, _SB_LOG_LEVEL(baseLevel));
        _SB_LOG_BLOCK_CLOSER();
        
        __SBDetermineLevels(support, baseLevel);
        __SBSaveLevels(&support->runChain, support->refLevels, baseLevel);
        
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
