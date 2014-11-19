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

#include <SBConfig.h>
#include <SBTypes.h>

#include "SBAssert.h"
#include "SBLog.h"

#include "SBCharType.h"
#include "SBBracketType.h"
#include "SBPairingLookup.h"

#include "SBBidiLink.h"
#include "SBLevelRun.h"

#include "SBBracketQueue.h"
#include "SBIsolatingRun.h"

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

static void _SBAttachLevelRunLinks(_SBIsolatingRunRef isolatingRun);
static void _SBAttachOriginalLinks(_SBIsolatingRunRef isolatingRun);

static _SBBidiLinkRef __SBResolveWeakTypes(_SBIsolatingRunRef isolatingRun);
static void __SBResolveBrackets(_SBIsolatingRunRef isolatingRun);
static void __SBResolveAvailableBracketPairs(_SBIsolatingRunRef isolatingRun);
static void __SBResolveNeutrals(_SBIsolatingRunRef isolatingRun);
static void __SBResolveImplicitLevels(_SBIsolatingRunRef isolatingRun);

SB_INTERNAL void _SBIsolatingRunInitialize(_SBIsolatingRunRef isolatingRun) {
    _SBBidiLinkMakeEmpty(&isolatingRun->_dummyLink);
    _SBBracketQueueInitialize(&isolatingRun->_bracketQueue);
}

SB_INTERNAL void _SBIsolatingRunResolve(_SBIsolatingRunRef isolatingRun) {
    _SBBidiLinkRef lastLink;
    _SBBidiLinkRef subsequentLink;

    _SB_LOG_BLOCK_OPENER("Identified Isolating Run");

    /* Attach level run links to form isolating run. */
    _SBAttachLevelRunLinks(isolatingRun);
    /* Save last subsequent link. */
    subsequentLink = isolatingRun->_lastLevelRun->subsequentLink;

    _SB_LOG_STATEMENT("Range", 1, _SB_LOG_RUN_RANGE(&isolatingRun->_dummyLink));
    _SB_LOG_STATEMENT("Types", 1, _SB_LOG_LINK_TYPES(&isolatingRun->_dummyLink));
    _SB_LOG_STATEMENT("Level", 1, _SB_LOG_LEVEL(_SBLevelRunGetLevel(isolatingRun->baseLevelRun)));
    _SB_LOG_STATEMENT("SOS", 1, _SB_LOG_CHAR_TYPE(isolatingRun->_sos));
    _SB_LOG_STATEMENT("EOS", 1, _SB_LOG_CHAR_TYPE(isolatingRun->_eos));

    /* Rules W1-W7 */
    lastLink = __SBResolveWeakTypes(isolatingRun);
    _SB_LOG_BLOCK_OPENER("Resolved Weak Types");
    _SB_LOG_STATEMENT("Types", 1, _SB_LOG_LINK_TYPES(&isolatingRun->_dummyLink));
    _SB_LOG_BLOCK_CLOSER();

    /* Rule N0 */
    __SBResolveBrackets(isolatingRun);
    _SB_LOG_BLOCK_OPENER("Resolved Brackets");
    _SB_LOG_STATEMENT("Types", 1, _SB_LOG_LINK_TYPES(&isolatingRun->_dummyLink));
    _SB_LOG_BLOCK_CLOSER();

    /* Rules N1, N2 */
    __SBResolveNeutrals(isolatingRun);
    _SB_LOG_BLOCK_OPENER("Resolved Neutrals");
    _SB_LOG_STATEMENT("Types", 1, _SB_LOG_LINK_TYPES(&isolatingRun->_dummyLink));
    _SB_LOG_BLOCK_CLOSER();

    /* Rules I1, I2 */
    __SBResolveImplicitLevels(isolatingRun);
    _SB_LOG_BLOCK_OPENER("Resolved Implicit Levels");
    _SB_LOG_STATEMENT("Levels", 1, _SB_LOG_LINK_LEVELS(&isolatingRun->_dummyLink));
    _SB_LOG_BLOCK_CLOSER();

    /* Re-attach original links. */
    _SBAttachOriginalLinks(isolatingRun);
    /* Attach new final link (of isolating run) with last subsequent link. */
    _SBBidiLinkReplaceNext(lastLink, subsequentLink);

    _SB_LOG_BLOCK_CLOSER();
}

SB_INTERNAL void _SBIsolatingRunInvalidate(_SBIsolatingRunRef isolatingRun) {
    _SBBracketQueueInvalidate(&isolatingRun->_bracketQueue);
}

static void _SBAttachLevelRunLinks(_SBIsolatingRunRef isolatingRun) {
    _SBLevelRunRef baseLevelRun;
    _SBLevelRunRef current;
    _SBLevelRunRef next;

    baseLevelRun = isolatingRun->baseLevelRun;
    _SBBidiLinkReplaceNext(&isolatingRun->_dummyLink, baseLevelRun->firstLink);

    /*
     * Iterate over level runs and attach their links to form an isolating run.
     */
    for (current = baseLevelRun; (next = current->next); current = next) {
        _SBBidiLinkReplaceNext(current->lastLink, next->firstLink);
    }
    _SBBidiLinkReplaceNext(current->lastLink, &isolatingRun->_dummyLink);

    isolatingRun->_lastLevelRun = current;
    isolatingRun->_sos = _SB_RUN_EXTREMA__GET_SOR(baseLevelRun->extrema);

    if (!_SB_RUN_KIND__IS_PARTIAL_ISOLATE(baseLevelRun->kind)) {
        isolatingRun->_eos = _SB_RUN_EXTREMA__GET_EOR(current->extrema);
    } else {
        SBLevel paragraphLevel = isolatingRun->paragraphLevel;
        SBLevel runLevel = _SBLevelRunGetLevel(baseLevelRun);
        SBLevel eosLevel = (runLevel > paragraphLevel ? runLevel : paragraphLevel);
        isolatingRun->_eos = ((eosLevel & 1) ? _SB_CHAR_TYPE__R : _SB_CHAR_TYPE__L);
    }
}

static void _SBAttachOriginalLinks(_SBIsolatingRunRef isolatingRun) {
    _SBLevelRunRef current;

    /*
     * Iterate over level runs and attach original subsequent links.
     */
    for (current = isolatingRun->baseLevelRun; current; current = current->next) {
        _SBBidiLinkReplaceNext(current->lastLink, current->subsequentLink);
    }
}

static _SBBidiLinkRef __SBResolveWeakTypes(_SBIsolatingRunRef isolatingRun) {
    _SBBidiLinkRef roller = &isolatingRun->_dummyLink;
    _SBBidiLinkRef link;

    _SBBidiLinkRef priorLink;
    _SBCharType sos;

    _SBCharType w1PriorType;
    _SBCharType w2StrongType;
    _SBCharType w4PriorType;
    _SBCharType w5PriorType;
    _SBCharType w7StrongType;

    priorLink = roller;
    sos = isolatingRun->_sos;

    w1PriorType = sos;
    w2StrongType = sos;

    for (link = roller->next; link != roller; link = link->next) {
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
            _SBBidiLinkMergeNext(priorLink);
        } else {
            priorLink = link;
        }
    }

    priorLink = roller;
    w4PriorType = sos;
    w5PriorType = sos;
    w7StrongType = sos;

    for (link = roller->next; link != roller; link = link->next) {
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
            _SBBidiLinkMergeNext(priorLink);
        } else {
            priorLink = link;
        }
    }

    return priorLink;
}

static void __SBResolveBrackets(_SBIsolatingRunRef isolatingRun) {
    SBUnichar *characters = isolatingRun->characters;
    _SBBracketQueueRef queue = &isolatingRun->_bracketQueue;
    _SBBidiLinkRef roller = &isolatingRun->_dummyLink;
    _SBBidiLinkRef link;

    _SBBidiLinkRef priorStrongLink;
    SBLevel runLevel;

    priorStrongLink = NULL;
    runLevel = _SBLevelRunGetLevel(isolatingRun->baseLevelRun);

    _SBBracketQueueReset(queue, SB_LEVEL_TO_EXACT_TYPE(runLevel));

    for (link = roller->next; link != roller; link = link->next) {
        SBUnichar ch;
        _SBCharType type;

        SBUnichar bracketValue;
        _SBBracketType bracketType;

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
                        __SBResolveAvailableBracketPairs(isolatingRun);
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

    __SBResolveAvailableBracketPairs(isolatingRun);
}

static void __SBResolveAvailableBracketPairs(_SBIsolatingRunRef isolatingRun) {
    _SBBracketQueueRef queue = &isolatingRun->_bracketQueue;

    SBLevel runLevel;
    _SBCharType embeddingDirection;
    _SBCharType oppositeDirection;

    runLevel = _SBLevelRunGetLevel(isolatingRun->baseLevelRun);
    embeddingDirection = SB_LEVEL_TO_EXACT_TYPE(runLevel);
    oppositeDirection = SB_LEVEL_TO_OPPOSITE_TYPE(runLevel);

    while (queue->count != 0) {
        _SBBidiLinkRef openingLink = _SBBracketQueueGetOpeningLink(queue);
        _SBBidiLinkRef closingLink = _SBBracketQueueGetClosingLink(queue);

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
                _SBBidiLinkRef priorStrongLink;
                _SBCharType priorStrongType;

                priorStrongLink = _SBBracketQueueGetPriorStrongLink(queue);

                if (priorStrongLink) {
                    _SBBidiLinkRef link;

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
                    priorStrongType = isolatingRun->_sos;
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
    _SBBidiLinkRef roller = &isolatingRun->_dummyLink;
    _SBBidiLinkRef link;

    SBLevel runLevel;
    _SBCharType strongType;
    _SBBidiLinkRef neutralLink;

    runLevel = _SBLevelRunGetLevel(isolatingRun->baseLevelRun);
    strongType = isolatingRun->_sos;
    neutralLink = NULL;

    for (link = roller->next; link != roller; link = link->next) {
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
                nextType = isolatingRun->_eos;
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
    _SBBidiLinkRef roller = &isolatingRun->_dummyLink;
    _SBBidiLinkRef link;

    SBLevel runLevel = _SBLevelRunGetLevel(isolatingRun->baseLevelRun);
    
    if ((runLevel & 1) == 0) {
        for (link = roller->next; link != roller; link = link->next) {
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
        for (link = roller->next; link != roller; link = link->next) {
            _SBCharType type = link->type;
            
            SBAssert(_SB_CHAR_TYPE__IS_STRONG_OR_NUMBER(type));
            
            /* Rule I2 */
            if (type != _SB_CHAR_TYPE__R) {
                link->level += 1;
            }
        }
    }
}
