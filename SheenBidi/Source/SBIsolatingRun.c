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
#include <SBTypes.h>

#include <stddef.h>

#include "SBAssert.h"
#include "SBBidiLink.h"
#include "SBBracketQueue.h"
#include "SBBracketType.h"
#include "SBCharType.h"
#include "SBLevelRun.h"
#include "SBLog.h"
#include "SBPairingLookup.h"
#include "SBIsolatingRun.h"

#define SB_LEVEL_TO_EXACT_TYPE(l)       \
(                                       \
   ((l) & 1)                            \
 ? SBCharTypeR                      \
 : SBCharTypeL                      \
)

#define SB_LEVEL_TO_OPPOSITE_TYPE(l)    \
(                                       \
 ((l) & 1)                              \
 ? SBCharTypeL                      \
 : SBCharTypeR                      \
)

static void _SBAttachLevelRunLinks(SBIsolatingRunRef isolatingRun);
static void _SBAttachOriginalLinks(SBIsolatingRunRef isolatingRun);

static SBBidiLinkRef _SBResolveWeakTypes(SBIsolatingRunRef isolatingRun);
static void _SBResolveBrackets(SBIsolatingRunRef isolatingRun);
static void _SBResolveAvailableBracketPairs(SBIsolatingRunRef isolatingRun);
static void _SBResolveNeutrals(SBIsolatingRunRef isolatingRun);
static void _SBResolveImplicitLevels(SBIsolatingRunRef isolatingRun);

SB_INTERNAL void SBIsolatingRunInitialize(SBIsolatingRunRef isolatingRun) {
    SBBidiLinkMakeEmpty(&isolatingRun->_dummyLink);
    SBBracketQueueInitialize(&isolatingRun->_bracketQueue);
}

SB_INTERNAL void SBIsolatingRunResolve(SBIsolatingRunRef isolatingRun) {
    SBBidiLinkRef lastLink;
    SBBidiLinkRef subsequentLink;

    SB_LOG_BLOCK_OPENER("Identified Isolating Run");

    /* Attach level run links to form isolating run. */
    _SBAttachLevelRunLinks(isolatingRun);
    /* Save last subsequent link. */
    subsequentLink = isolatingRun->_lastLevelRun->subsequentLink;

    SB_LOG_STATEMENT("Range", 1, SB_LOG_RUN_RANGE(&isolatingRun->_dummyLink));
    SB_LOG_STATEMENT("Types", 1, SB_LOG_LINK_TYPES(&isolatingRun->_dummyLink));
    SB_LOG_STATEMENT("Level", 1, SB_LOG_LEVEL(SBLevelRunGetLevel(isolatingRun->baseLevelRun)));
    SB_LOG_STATEMENT("SOS", 1, SB_LOG_CHAR_TYPE(isolatingRun->_sos));
    SB_LOG_STATEMENT("EOS", 1, SB_LOG_CHAR_TYPE(isolatingRun->_eos));

    /* Rules W1-W7 */
    lastLink = _SBResolveWeakTypes(isolatingRun);
    SB_LOG_BLOCK_OPENER("Resolved Weak Types");
    SB_LOG_STATEMENT("Types", 1, SB_LOG_LINK_TYPES(&isolatingRun->_dummyLink));
    SB_LOG_BLOCK_CLOSER();

    /* Rule N0 */
    _SBResolveBrackets(isolatingRun);
    SB_LOG_BLOCK_OPENER("Resolved Brackets");
    SB_LOG_STATEMENT("Types", 1, SB_LOG_LINK_TYPES(&isolatingRun->_dummyLink));
    SB_LOG_BLOCK_CLOSER();

    /* Rules N1, N2 */
    _SBResolveNeutrals(isolatingRun);
    SB_LOG_BLOCK_OPENER("Resolved Neutrals");
    SB_LOG_STATEMENT("Types", 1, SB_LOG_LINK_TYPES(&isolatingRun->_dummyLink));
    SB_LOG_BLOCK_CLOSER();

    /* Rules I1, I2 */
    _SBResolveImplicitLevels(isolatingRun);
    SB_LOG_BLOCK_OPENER("Resolved Implicit Levels");
    SB_LOG_STATEMENT("Levels", 1, SB_LOG_LINK_LEVELS(&isolatingRun->_dummyLink));
    SB_LOG_BLOCK_CLOSER();

    /* Re-attach original links. */
    _SBAttachOriginalLinks(isolatingRun);
    /* Attach new final link (of isolating run) with last subsequent link. */
    SBBidiLinkReplaceNext(lastLink, subsequentLink);

    SB_LOG_BLOCK_CLOSER();
}

SB_INTERNAL void SBIsolatingRunFinalize(SBIsolatingRunRef isolatingRun) {
    SBBracketQueueFinalize(&isolatingRun->_bracketQueue);
}

static void _SBAttachLevelRunLinks(SBIsolatingRunRef isolatingRun) {
    SBLevelRunRef baseLevelRun;
    SBLevelRunRef current;
    SBLevelRunRef next;

    baseLevelRun = isolatingRun->baseLevelRun;
    SBBidiLinkReplaceNext(&isolatingRun->_dummyLink, baseLevelRun->firstLink);

    /* Iterate over level runs and attach their links to form an isolating run. */
    for (current = baseLevelRun; (next = current->next); current = next) {
        SBBidiLinkReplaceNext(current->lastLink, next->firstLink);
    }
    SBBidiLinkReplaceNext(current->lastLink, &isolatingRun->_dummyLink);

    isolatingRun->_lastLevelRun = current;
    isolatingRun->_sos = SBRunExtrema_SOR(baseLevelRun->extrema);

    if (!SB_RUN_KIND__IS_PARTIAL_ISOLATE(baseLevelRun->kind)) {
        isolatingRun->_eos = SBRunExtrema_EOR(current->extrema);
    } else {
        SBLevel paragraphLevel = isolatingRun->paragraphLevel;
        SBLevel runLevel = SBLevelRunGetLevel(baseLevelRun);
        SBLevel eosLevel = (runLevel > paragraphLevel ? runLevel : paragraphLevel);
        isolatingRun->_eos = ((eosLevel & 1) ? SBCharTypeR : SBCharTypeL);
    }
}

static void _SBAttachOriginalLinks(SBIsolatingRunRef isolatingRun) {
    SBLevelRunRef current;

    /* Iterate over level runs and attach original subsequent links. */
    for (current = isolatingRun->baseLevelRun; current; current = current->next) {
        SBBidiLinkReplaceNext(current->lastLink, current->subsequentLink);
    }
}

static SBBidiLinkRef _SBResolveWeakTypes(SBIsolatingRunRef isolatingRun) {
    SBBidiLinkRef roller = &isolatingRun->_dummyLink;
    SBBidiLinkRef link;

    SBBidiLinkRef priorLink;
    SBCharType sos;

    SBCharType w1PriorType;
    SBCharType w2StrongType;
    SBCharType w4PriorType;
    SBCharType w5PriorType;
    SBCharType w7StrongType;

    priorLink = roller;
    sos = isolatingRun->_sos;

    w1PriorType = sos;
    w2StrongType = sos;

    for (link = roller->next; link != roller; link = link->next) {
        SBCharType type = link->type;

        /* Rule W1 */
        if (type == SBCharTypeNSM) {
            /* Change the 'type' variable as well because it can be EN on which W2 depends. */
            link->type = type = (SBCharTypeIsIsolate(w1PriorType) ? SBCharTypeON : w1PriorType);
        }
        w1PriorType = type;

        /* Rule W2 */
        if (type == SBCharTypeEN) {
            if (w2StrongType == SBCharTypeAL) {
                link->type = SBCharTypeAN;
            }
        }
        /*
         * Rule W3
         * NOTE: It is safe to apply W3 in 'else-if' statement because it only depends on type AL.
         *       Even if W2 changes EN to AN, there won't be any harm.
         */
        else if (type == SBCharTypeAL) {
            link->type = SBCharTypeR;
        }

        if (SBCharTypeIsStrong(type)) {
            /* Save the strong type as it is checked in W2. */
            w2StrongType = type;
        }

        if (type != SBCharTypeON && priorLink->type == type) {
            SBBidiLinkMergeNext(priorLink);
        } else {
            priorLink = link;
        }
    }

    priorLink = roller;
    w4PriorType = sos;
    w5PriorType = sos;
    w7StrongType = sos;

    for (link = roller->next; link != roller; link = link->next) {
        SBCharType type = link->type;
        SBCharType nextType = link->next->type;

        /* Rule W4 */
        if (link->length == 1
            && SBCharTypeIsNumberSeparator(type)
            && SBCharTypeIsNumber(w4PriorType)
            && (w4PriorType == nextType)
            && (w4PriorType == SBCharTypeEN || type == SBCharTypeCS))
        {
            /* Change the current type as well because it can be EN on which W5 depends. */
            link->type = type = w4PriorType;
        }
        w4PriorType = type;

        /* Rule W5 */
        if (type == SBCharTypeET && (w5PriorType == SBCharTypeEN || nextType == SBCharTypeEN)) {
            /* Change the current type as well because it is EN on which W7 depends. */
            link->type = type = SBCharTypeEN;
        }
        w5PriorType = type;

        switch (type) {
        /* Rule W6 */
        case SBCharTypeET:
        case SBCharTypeCS:
        case SBCharTypeES:
            link->type = SBCharTypeON;
            break;

        /*
         * Rule W7
         * NOTE: W7 is expected to be applied after W6. However this is not the case here. The
         *       reason is that W6 can only create the type ON which is not tested in W7 by any
         *       means. So it won't affect the algorithm.
         */
        case SBCharTypeEN:
            if (w7StrongType == SBCharTypeL) {
                link->type = SBCharTypeL;
            }
            break;

        /*
         * Save the strong type for W7.
         * NOTE: The strong type is expected to be saved after applying W7 because W7 itself creates
         *       a strong type. However the strong type being saved here is based on the type after
         *       W5. This won't effect the algorithm because a single link contains all consecutive
         *       EN types. This means that even if W7 creates a strong type, it will be saved in
         *       next iteration.
         */
        case SBCharTypeL:
        case SBCharTypeR:
            w7StrongType = type;
            break;
        }

        if (type != SBCharTypeON && priorLink->type == type) {
            SBBidiLinkMergeNext(priorLink);
        } else {
            priorLink = link;
        }
    }

    return priorLink;
}

static void _SBResolveBrackets(SBIsolatingRunRef isolatingRun) {
    SBCodepoint *codepoints = isolatingRun->codepoints;
    SBBracketQueueRef queue = &isolatingRun->_bracketQueue;
    SBBidiLinkRef roller = &isolatingRun->_dummyLink;
    SBBidiLinkRef link;

    SBBidiLinkRef priorStrongLink;
    SBLevel runLevel;

    priorStrongLink = NULL;
    runLevel = SBLevelRunGetLevel(isolatingRun->baseLevelRun);

    SBBracketQueueReset(queue, SB_LEVEL_TO_EXACT_TYPE(runLevel));

    for (link = roller->next; link != roller; link = link->next) {
        SBCodepoint ch;
        SBCharType type;

        SBCodepoint bracketValue;
        SBBracketType bracketType;

        type = link->type;

        switch (type) {
        case SBCharTypeON:
            ch = codepoints[link->offset];
            bracketValue = SBPairingDetermineBracketPair(ch, &bracketType);

            switch (bracketType) {
            case SBBracketTypeOpen:
                SBBracketQueueEnqueue(queue, priorStrongLink, link, ch);
                break;

            case SBBracketTypeClose:
                if (queue->count != 0) {
                    SBBracketQueueClosePair(queue, link, bracketValue);

                    if (SBBracketQueueShouldDequeue(queue)) {
                        _SBResolveAvailableBracketPairs(isolatingRun);
                    }
                }
                break;
            }
            break;

        case SBCharTypeEN:
        case SBCharTypeAN:
            type = SBCharTypeR;

        case SBCharTypeR:
        case SBCharTypeL:
            if (queue->count != 0) {
                SBBracketQueueSetStrongType(queue, type);
            }

            priorStrongLink = link;
            break;
        }
    }

    _SBResolveAvailableBracketPairs(isolatingRun);
}

static void _SBResolveAvailableBracketPairs(SBIsolatingRunRef isolatingRun) {
    SBBracketQueueRef queue = &isolatingRun->_bracketQueue;

    SBLevel runLevel;
    SBCharType embeddingDirection;
    SBCharType oppositeDirection;

    runLevel = SBLevelRunGetLevel(isolatingRun->baseLevelRun);
    embeddingDirection = SB_LEVEL_TO_EXACT_TYPE(runLevel);
    oppositeDirection = SB_LEVEL_TO_OPPOSITE_TYPE(runLevel);

    while (queue->count != 0) {
        SBBidiLinkRef openingLink = SBBracketQueueGetOpeningLink(queue);
        SBBidiLinkRef closingLink = SBBracketQueueGetClosingLink(queue);

        if (openingLink && closingLink) {
            SBCharType innerStrongType;
            SBCharType pairType;

            innerStrongType = SBBracketQueueGetStrongType(queue);

            /* Rule: N0.b */
            if (innerStrongType == embeddingDirection) {
                pairType = innerStrongType;
            }
            /* Rule: N0.c */
            else if (innerStrongType == oppositeDirection) {
                SBBidiLinkRef priorStrongLink;
                SBCharType priorStrongType;

                priorStrongLink = SBBracketQueueGetPriorStrongLink(queue);

                if (priorStrongLink) {
                    SBBidiLinkRef link;

                    priorStrongType = priorStrongLink->type;
                    if (SBCharTypeIsNumber(priorStrongType)) {
                        priorStrongType = SBCharTypeR;
                    }

                    link = priorStrongLink->next;

                    while (link != openingLink) {
                        SBCharType type = link->type;
                        if (type == SBCharTypeL || type == SBCharTypeR) {
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
                pairType = SBCharTypeNil;
            }

            if (pairType != SBCharTypeNil) {
                /* Do the substitution */
                openingLink->type = pairType;
                closingLink->type = pairType;
            }
        }

        SBBracketQueueDequeue(queue);
    }
}

static void _SBResolveNeutrals(SBIsolatingRunRef isolatingRun) {
    SBBidiLinkRef roller = &isolatingRun->_dummyLink;
    SBBidiLinkRef link;

    SBLevel runLevel;
    SBCharType strongType;
    SBBidiLinkRef neutralLink;

    runLevel = SBLevelRunGetLevel(isolatingRun->baseLevelRun);
    strongType = isolatingRun->_sos;
    neutralLink = NULL;

    for (link = roller->next; link != roller; link = link->next) {
        SBCharType type = link->type;
        SBCharType nextType;

        SBAssert(SBCharTypeIsStrongOrNumber(type) || SBCharTypeIsNeutralOrIsolate(type));

        switch (type) {
        case SBCharTypeL:
            strongType = SBCharTypeL;
            break;

        case SBCharTypeR:
        case SBCharTypeEN:
        case SBCharTypeAN:
            strongType = SBCharTypeR;
            break;

        case SBCharTypeB:                           
        case SBCharTypeS:
        case SBCharTypeWS:
        case SBCharTypeON:
        case SBCharTypeLRI:
        case SBCharTypeRLI:                         
        case SBCharTypeFSI:
        case SBCharTypePDI:
            if (!neutralLink) {
                neutralLink = link;
            }

            nextType = link->next->type;
            if (SBCharTypeIsNumber(nextType)) {
                nextType = SBCharTypeR;
            } else if (nextType == SBCharTypeNil) {
                nextType = isolatingRun->_eos;
            }

            if (SBCharTypeIsStrong(nextType)) {
                /* Rules N1, N2 */
                SBCharType resolvedType = (strongType == nextType
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

static void _SBResolveImplicitLevels(SBIsolatingRunRef isolatingRun) {
    SBBidiLinkRef roller = &isolatingRun->_dummyLink;
    SBBidiLinkRef link;

    SBLevel runLevel = SBLevelRunGetLevel(isolatingRun->baseLevelRun);
    
    if ((runLevel & 1) == 0) {
        for (link = roller->next; link != roller; link = link->next) {
            SBCharType type = link->type;
            
            SBAssert(SBCharTypeIsStrongOrNumber(type));
            
            /* Rule I1 */
            if (type == SBCharTypeR) {
                link->level += 1;
            } else if (type != SBCharTypeL) {
                link->level += 2;
            }
        }
    } else {
        for (link = roller->next; link != roller; link = link->next) {
            SBCharType type = link->type;
            
            SBAssert(SBCharTypeIsStrongOrNumber(type));
            
            /* Rule I2 */
            if (type != SBCharTypeR) {
                link->level += 1;
            }
        }
    }
}
