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

#include "SBAssert.h"
#include "SBBase.h"
#include "SBBidiChain.h"
#include "SBBracketQueue.h"
#include "SBBracketType.h"
#include "SBLevelRun.h"
#include "SBLog.h"
#include "SBPairingLookup.h"
#include "SBIsolatingRun.h"

static void _SBAttachLevelRunLinks(SBIsolatingRunRef isolatingRun);
static void _SBAttachOriginalLinks(SBIsolatingRunRef isolatingRun);

static SBBidiLink _SBResolveWeakTypes(SBIsolatingRunRef isolatingRun);
static void _SBResolveBrackets(SBIsolatingRunRef isolatingRun);
static void _SBResolveAvailableBracketPairs(SBIsolatingRunRef isolatingRun);
static void _SBResolveNeutrals(SBIsolatingRunRef isolatingRun);
static void _SBResolveImplicitLevels(SBIsolatingRunRef isolatingRun);

SB_INTERNAL void SBIsolatingRunInitialize(SBIsolatingRunRef isolatingRun)
{
    SBBracketQueueInitialize(&isolatingRun->_bracketQueue);
}

SB_INTERNAL void SBIsolatingRunResolve(SBIsolatingRunRef isolatingRun)
{
    SBBidiLink lastLink;
    SBBidiLink subsequentLink;

    SB_LOG_BLOCK_OPENER("Identified Isolating Run");

    /* Attach level run links to form isolating run. */
    _SBAttachLevelRunLinks(isolatingRun);
    /* Save last subsequent link. */
    subsequentLink = isolatingRun->_lastLevelRun->subsequentLink;

    SB_LOG_STATEMENT("Range", 1, SB_LOG_RUN_RANGE(isolatingRun));
    SB_LOG_STATEMENT("Types", 1, SB_LOG_RUN_TYPES(isolatingRun));
    SB_LOG_STATEMENT("Level", 1, SB_LOG_LEVEL(isolatingRun->baseLevelRun->level));
    SB_LOG_STATEMENT("SOS", 1, SB_LOG_BIDI_TYPE(isolatingRun->_sos));
    SB_LOG_STATEMENT("EOS", 1, SB_LOG_BIDI_TYPE(isolatingRun->_eos));

    /* Rules W1-W7 */
    lastLink = _SBResolveWeakTypes(isolatingRun);
    SB_LOG_BLOCK_OPENER("Resolved Weak Types");
    SB_LOG_STATEMENT("Types", 1, SB_LOG_RUN_TYPES(isolatingRun));
    SB_LOG_BLOCK_CLOSER();

    /* Rule N0 */
    _SBResolveBrackets(isolatingRun);
    SB_LOG_BLOCK_OPENER("Resolved Brackets");
    SB_LOG_STATEMENT("Types", 1, SB_LOG_RUN_TYPES(isolatingRun));
    SB_LOG_BLOCK_CLOSER();

    /* Rules N1, N2 */
    _SBResolveNeutrals(isolatingRun);
    SB_LOG_BLOCK_OPENER("Resolved Neutrals");
    SB_LOG_STATEMENT("Types", 1, SB_LOG_RUN_TYPES(isolatingRun));
    SB_LOG_BLOCK_CLOSER();

    /* Rules I1, I2 */
    _SBResolveImplicitLevels(isolatingRun);
    SB_LOG_BLOCK_OPENER("Resolved Implicit Levels");
    SB_LOG_STATEMENT("Levels", 1, SB_LOG_RUN_LEVELS(isolatingRun));
    SB_LOG_BLOCK_CLOSER();

    /* Re-attach original links. */
    _SBAttachOriginalLinks(isolatingRun);
    /* Attach new final link (of isolating run) with last subsequent link. */
    SBBidiChainSetNext(isolatingRun->bidiChain, lastLink, subsequentLink);

    SB_LOG_BLOCK_CLOSER();
}

SB_INTERNAL void SBIsolatingRunFinalize(SBIsolatingRunRef isolatingRun)
{
    SBBracketQueueFinalize(&isolatingRun->_bracketQueue);
}

static void _SBAttachLevelRunLinks(SBIsolatingRunRef isolatingRun)
{
    SBBidiChainRef chain = isolatingRun->bidiChain;
    SBLevelRunRef baseLevelRun = isolatingRun->baseLevelRun;
    SBLevelRunRef current;
    SBLevelRunRef next;

    isolatingRun->_originalLink = SBBidiChainGetNext(chain, chain->roller);
    SBBidiChainSetNext(chain, chain->roller, baseLevelRun->firstLink);

    /* Iterate over level runs and attach their links to form an isolating run. */
    for (current = baseLevelRun; (next = current->next); current = next) {
        SBBidiChainSetNext(chain, current->lastLink, next->firstLink);
    }
    SBBidiChainSetNext(chain, current->lastLink, chain->roller);

    isolatingRun->_lastLevelRun = current;
    isolatingRun->_sos = SBRunExtrema_SOR(baseLevelRun->extrema);

    if (!SBRunKindIsPartialIsolate(baseLevelRun->kind)) {
        isolatingRun->_eos = SBRunExtrema_EOR(current->extrema);
    } else {
        SBLevel paragraphLevel = isolatingRun->paragraphLevel;
        SBLevel runLevel = baseLevelRun->level;
        SBLevel eosLevel = (runLevel > paragraphLevel ? runLevel : paragraphLevel);
        isolatingRun->_eos = ((eosLevel & 1) ? SBBidiTypeR : SBBidiTypeL);
    }
}

static void _SBAttachOriginalLinks(SBIsolatingRunRef isolatingRun)
{
    SBBidiChainRef chain = isolatingRun->bidiChain;
    SBLevelRunRef current;

    SBBidiChainSetNext(chain, chain->roller, isolatingRun->_originalLink);

    /* Iterate over level runs and attach original subsequent links. */
    for (current = isolatingRun->baseLevelRun; current; current = current->next) {
        SBBidiChainSetNext(chain, current->lastLink, current->subsequentLink);
    }
}

static SBBidiLink _SBResolveWeakTypes(SBIsolatingRunRef isolatingRun)
{
    SBBidiChainRef chain = isolatingRun->bidiChain;
    SBBidiLink roller = chain->roller;
    SBBidiLink link;

    SBBidiLink priorLink;
    SBBidiType sos;

    SBBidiType w1PriorType;
    SBBidiType w2StrongType;
    SBBidiType w4PriorType;
    SBBidiType w5PriorType;
    SBBidiType w7StrongType;

    priorLink = roller;
    sos = isolatingRun->_sos;

    w1PriorType = sos;
    w2StrongType = sos;

    SBBidiChainForEach(chain, roller, link) {
        SBBidiType type = SBBidiChainGetType(chain, link);
        SBBoolean forceMerge = SBFalse;

        /* Rule W1 */
        if (type == SBBidiTypeNSM) {
            /* Change the 'type' variable as well because it can be EN on which W2 depends. */
            type = (SBBidiTypeIsIsolate(w1PriorType) ? SBBidiTypeON : w1PriorType);
            SBBidiChainSetType(chain, link, type);

            /* Fix for 3rd point of rule N0. */
            if (w1PriorType == SBBidiTypeON) {
                forceMerge = SBTrue;
            }
        }
        w1PriorType = type;

        /* Rule W2 */
        if (type == SBBidiTypeEN) {
            if (w2StrongType == SBBidiTypeAL) {
                SBBidiChainSetType(chain, link, SBBidiTypeAN);
            }
        }
        /*
         * Rule W3
         * NOTE: It is safe to apply W3 in 'else-if' statement because it only depends on type AL.
         *       Even if W2 changes EN to AN, there won't be any harm.
         */
        else if (type == SBBidiTypeAL) {
            SBBidiChainSetType(chain, link, SBBidiTypeR);
        }

        if (SBBidiTypeIsStrong(type)) {
            /* Save the strong type as it is checked in W2. */
            w2StrongType = type;
        }

        if ((type != SBBidiTypeON && SBBidiChainGetType(chain, priorLink) == type) || forceMerge) {
            SBBidiChainAbandonNext(chain, priorLink);
        } else {
            priorLink = link;
        }
    }

    priorLink = roller;
    w4PriorType = sos;
    w5PriorType = sos;
    w7StrongType = sos;

    SBBidiChainForEach(chain, roller, link) {
        SBBidiType type = SBBidiChainGetType(chain, link);
        SBBidiType nextType = SBBidiChainGetType(chain, SBBidiChainGetNext(chain, link));

        /* Rule W4 */
        if (SBBidiChainIsSingle(chain, link)
            && SBBidiTypeIsNumberSeparator(type)
            && SBBidiTypeIsNumber(w4PriorType)
            && (w4PriorType == nextType)
            && (w4PriorType == SBBidiTypeEN || type == SBBidiTypeCS))
        {
            /* Change the current type as well because it can be EN on which W5 depends. */
            type = w4PriorType;
            SBBidiChainSetType(chain, link, type);
        }
        w4PriorType = type;

        /* Rule W5 */
        if (type == SBBidiTypeET && (w5PriorType == SBBidiTypeEN || nextType == SBBidiTypeEN)) {
            /* Change the current type as well because it is EN on which W7 depends. */
            type = SBBidiTypeEN;
            SBBidiChainSetType(chain, link, type);
        }
        w5PriorType = type;

        switch (type) {
        /* Rule W6 */
        case SBBidiTypeET:
        case SBBidiTypeCS:
        case SBBidiTypeES:
            SBBidiChainSetType(chain, link, SBBidiTypeON);
            break;

        /*
         * Rule W7
         * NOTE: W7 is expected to be applied after W6. However this is not the case here. The
         *       reason is that W6 can only create the type ON which is not tested in W7 by any
         *       means. So it won't affect the algorithm.
         */
        case SBBidiTypeEN:
            if (w7StrongType == SBBidiTypeL) {
                SBBidiChainSetType(chain, link, SBBidiTypeL);
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
        case SBBidiTypeL:
        case SBBidiTypeR:
            w7StrongType = type;
            break;
        }

        if (type != SBBidiTypeON && SBBidiChainGetType(chain, priorLink) == type) {
            SBBidiChainAbandonNext(chain, priorLink);
        } else {
            priorLink = link;
        }
    }

    return priorLink;
}

static void _SBResolveBrackets(SBIsolatingRunRef isolatingRun)
{
    const SBCodepointSequence *sequence = isolatingRun->codepointSequence;
    SBUInteger paragraphOffset = isolatingRun->paragraphOffset;
    SBBracketQueueRef queue = &isolatingRun->_bracketQueue;
    SBBidiChainRef chain = isolatingRun->bidiChain;
    SBBidiLink roller = chain->roller;
    SBBidiLink link;

    SBBidiLink priorStrongLink;
    SBLevel runLevel;

    priorStrongLink = SBBidiLinkNone;
    runLevel = isolatingRun->baseLevelRun->level;

    SBBracketQueueReset(queue, SBLevelAsNormalBidiType(runLevel));

    SBBidiChainForEach(chain, roller, link) {
        SBUInteger stringIndex;
        SBCodepoint codepoint;
        SBBidiType type;

        SBCodepoint bracketValue;
        SBBracketType bracketType;

        type = SBBidiChainGetType(chain, link);

        switch (type) {
        case SBBidiTypeON:
            stringIndex = SBBidiChainGetOffset(chain, link) + paragraphOffset;
            codepoint = SBCodepointSequenceGetCodepointAt(sequence, &stringIndex);
            bracketValue = SBPairingDetermineBracketPair(codepoint, &bracketType);

            switch (bracketType) {
            case SBBracketTypeOpen:
                if (queue->count < SBBracketQueueGetMaxCapacity()) {
                    SBBracketQueueEnqueue(queue, priorStrongLink, link, bracketValue);
                } else {
                    goto Resolve;
                }
                break;

            case SBBracketTypeClose:
                if (queue->count != 0) {
                    SBBracketQueueClosePair(queue, link, codepoint);

                    if (SBBracketQueueShouldDequeue(queue)) {
                        _SBResolveAvailableBracketPairs(isolatingRun);
                    }
                }
                break;
            }
            break;

        case SBBidiTypeEN:
        case SBBidiTypeAN:
            type = SBBidiTypeR;

        case SBBidiTypeR:
        case SBBidiTypeL:
            if (queue->count != 0) {
                SBBracketQueueSetStrongType(queue, type);
            }

            priorStrongLink = link;
            break;
        }
    }

Resolve:
    _SBResolveAvailableBracketPairs(isolatingRun);
}

static void _SBResolveAvailableBracketPairs(SBIsolatingRunRef isolatingRun)
{
    SBBracketQueueRef queue = &isolatingRun->_bracketQueue;
    SBBidiChainRef chain = isolatingRun->bidiChain;

    SBLevel runLevel;
    SBBidiType embeddingDirection;
    SBBidiType oppositeDirection;

    runLevel = isolatingRun->baseLevelRun->level;
    embeddingDirection = SBLevelAsNormalBidiType(runLevel);
    oppositeDirection = SBLevelAsOppositeBidiType(runLevel);

    while (queue->count != 0) {
        SBBidiLink openingLink = SBBracketQueueGetOpeningLink(queue);
        SBBidiLink closingLink = SBBracketQueueGetClosingLink(queue);

        if ((openingLink != SBBidiLinkNone) && (closingLink != SBBidiLinkNone)) {
            SBBidiType innerStrongType;
            SBBidiType pairType;

            innerStrongType = SBBracketQueueGetStrongType(queue);

            /* Rule: N0.b */
            if (innerStrongType == embeddingDirection) {
                pairType = innerStrongType;
            }
            /* Rule: N0.c */
            else if (innerStrongType == oppositeDirection) {
                SBBidiLink priorStrongLink;
                SBBidiType priorStrongType;

                priorStrongLink = SBBracketQueueGetPriorStrongLink(queue);

                if (priorStrongLink != SBBidiLinkNone) {
                    SBBidiLink link;

                    priorStrongType = SBBidiChainGetType(chain, priorStrongLink);
                    if (SBBidiTypeIsNumber(priorStrongType)) {
                        priorStrongType = SBBidiTypeR;
                    }

                    link = SBBidiChainGetNext(chain, priorStrongLink);

                    while (link != openingLink) {
                        SBBidiType type = SBBidiChainGetType(chain, link);
                        if (type == SBBidiTypeL || type == SBBidiTypeR) {
                            priorStrongType = type;
                        }

                        link = SBBidiChainGetNext(chain, link);
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
                pairType = SBBidiTypeNil;
            }

            if (pairType != SBBidiTypeNil) {
                /* Do the substitution */
                SBBidiChainSetType(chain, openingLink, pairType);
                SBBidiChainSetType(chain, closingLink, pairType);
            }
        }

        SBBracketQueueDequeue(queue);
    }
}

static void _SBResolveNeutrals(SBIsolatingRunRef isolatingRun)
{
    SBBidiChainRef chain = isolatingRun->bidiChain;
    SBBidiLink roller = chain->roller;
    SBBidiLink link;

    SBLevel runLevel;
    SBBidiType strongType;
    SBBidiLink neutralLink;

    runLevel = isolatingRun->baseLevelRun->level;
    strongType = isolatingRun->_sos;
    neutralLink = SBBidiLinkNone;

    SBBidiChainForEach(chain, roller, link) {
        SBBidiType type = SBBidiChainGetType(chain, link);
        SBBidiType nextType;

        SBAssert(SBBidiTypeIsStrongOrNumber(type) || SBBidiTypeIsNeutralOrIsolate(type));

        switch (type) {
        case SBBidiTypeL:
            strongType = SBBidiTypeL;
            break;

        case SBBidiTypeR:
        case SBBidiTypeEN:
        case SBBidiTypeAN:
            strongType = SBBidiTypeR;
            break;

        case SBBidiTypeB:                           
        case SBBidiTypeS:
        case SBBidiTypeWS:
        case SBBidiTypeON:
        case SBBidiTypeLRI:
        case SBBidiTypeRLI:                         
        case SBBidiTypeFSI:
        case SBBidiTypePDI:
            if (neutralLink == SBBidiLinkNone) {
                neutralLink = link;
            }

            nextType = SBBidiChainGetType(chain, SBBidiChainGetNext(chain, link));
            if (SBBidiTypeIsNumber(nextType)) {
                nextType = SBBidiTypeR;
            } else if (nextType == SBBidiTypeNil) {
                nextType = isolatingRun->_eos;
            }

            if (SBBidiTypeIsStrong(nextType)) {
                /* Rules N1, N2 */
                SBBidiType resolvedType = (strongType == nextType
                                           ? strongType
                                           : SBLevelAsNormalBidiType(runLevel));

                do {
                    SBBidiChainSetType(chain, neutralLink, resolvedType);
                    neutralLink = SBBidiChainGetNext(chain, neutralLink);
                } while (neutralLink != SBBidiChainGetNext(chain, link));

                neutralLink = SBBidiLinkNone;
            }
            break;
        }
    }
}

static void _SBResolveImplicitLevels(SBIsolatingRunRef isolatingRun)
{
    SBBidiChainRef chain = isolatingRun->bidiChain;
    SBBidiLink roller = chain->roller;
    SBBidiLink link;

    SBLevel runLevel = isolatingRun->baseLevelRun->level;
    
    if ((runLevel & 1) == 0) {
        SBBidiChainForEach(chain, roller, link) {
            SBBidiType type = SBBidiChainGetType(chain, link);
            SBLevel level = SBBidiChainGetLevel(chain, link);
            
            SBAssert(SBBidiTypeIsStrongOrNumber(type));
            
            /* Rule I1 */
            if (type == SBBidiTypeR) {
                SBBidiChainSetLevel(chain, link, level + 1);
            } else if (type != SBBidiTypeL) {
                SBBidiChainSetLevel(chain, link, level + 2);
            }
        }
    } else {
        SBBidiChainForEach(chain, roller, link) {
            SBBidiType type = SBBidiChainGetType(chain, link);
            SBLevel level = SBBidiChainGetLevel(chain, link);
            
            SBAssert(SBBidiTypeIsStrongOrNumber(type));
            
            /* Rule I2 */
            if (type != SBBidiTypeR) {
                SBBidiChainSetLevel(chain, link, level + 1);
            }
        }
    }
}
