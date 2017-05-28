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

#ifdef SB_CONFIG_LOG

#include "SBBase.h"
#include "SBBidiChain.h"
#include "SBCharType.h"
#include "SBIsolatingRun.h"
#include "SBLog.h"

int _SBLogPosition = 0;

SB_INTERNAL void _SBPrintBaseLevel(SBLevel baseLevel)
{
    switch (baseLevel) {
    case SBLevelDefaultLTR:
        SB_LOG_STRING("Auto-LTR");
        break;

    case SBLevelDefaultRTL:
        SB_LOG_STRING("Auto-RTL");
        break;

    case 0:
        SB_LOG_STRING("LTR");
        break;

    case 1:
        SB_LOG_STRING("RTL");
        break;

    default:
        SB_LOG(("Level - %d", baseLevel));
        break;
    }
}

SB_INTERNAL void _SBPrintCharType(SBCharType type)
{
    switch (type) {
    case SBCharTypeNil:
        SB_LOG_STRING("Nil");
        break;

    case SBCharTypeL:
        SB_LOG_STRING("L");
        break;

    case SBCharTypeR:
        SB_LOG_STRING("R");
        break;

    case SBCharTypeAL:
        SB_LOG_STRING("AL");
        break;

    case SBCharTypeEN:
        SB_LOG_STRING("EN");
        break;

    case SBCharTypeES:
        SB_LOG_STRING("ES");
        break;

    case SBCharTypeET:
        SB_LOG_STRING("EN");
        break;

    case SBCharTypeAN:
        SB_LOG_STRING("AN");
        break;

    case SBCharTypeCS:
        SB_LOG_STRING("CS");
        break;

    case SBCharTypeNSM:
        SB_LOG_STRING("NSM");
        break;

    case SBCharTypeBN:
        SB_LOG_STRING("BN");
        break;

    case SBCharTypeB:
        SB_LOG_STRING("B");
        break;

    case SBCharTypeS:
        SB_LOG_STRING("S");
        break;

    case SBCharTypeWS:
        SB_LOG_STRING("WS");
        break;

    case SBCharTypeON:
        SB_LOG_STRING("ON");
        break;

    case SBCharTypeLRE:
        SB_LOG_STRING("LRE");
        break;

    case SBCharTypeRLE:
        SB_LOG_STRING("RLE");
        break;

    case SBCharTypeLRO:
        SB_LOG_STRING("LRO");
        break;

    case SBCharTypeRLO:
        SB_LOG_STRING("RLO");
        break;

    case SBCharTypePDF:
        SB_LOG_STRING("PDF");
        break;

    case SBCharTypeLRI:
        SB_LOG_STRING("LRI");
        break;

    case SBCharTypeRLI:
        SB_LOG_STRING("RLI");
        break;

    case SBCharTypeFSI:
        SB_LOG_STRING("FSI");
        break;

    case SBCharTypePDI:
        SB_LOG_STRING("PDI");
        break;
    }
}

SB_INTERNAL void _SBPrintCodepointSequence(const SBCodepointSequence *codepointSequence)
{
    SBUInteger stringIndex = 0;
    SBCodepoint codepoint;

    while ((codepoint = SBCodepointSequenceGetCodepointAt(codepointSequence, &stringIndex)) != SBCodepointInvalid) {
        SB_LOG(("%04X ", codepoint));
    }
}

SB_INTERNAL void _SBPrintCharTypesArray(SBCharType *types, SBUInteger length)
{
    SBUInteger index;

    for (index = 0; index < length; ++index) {
        SB_LOG_CHAR_TYPE(types[index]);
        SB_LOG_DIVIDER(1);
    }
}

SB_INTERNAL void _SBPrintLevelsArray(SBLevel *levels, SBUInteger length)
{
    SBUInteger index;

    for (index = 0; index < length; ++index) {
        SB_LOG_LEVEL(levels[index]);
        SB_LOG_DIVIDER(1);
    }
}

typedef struct {
    void *object;
    SBBidiLink link;
    SBUInteger length;
} _SBIsolatingContext;

typedef void (*_SBIsolatingConsumer)(SBIsolatingRunRef isolatingRun, _SBIsolatingContext *context);

SB_INTERNAL void _SBIsolatingRunForEach(SBIsolatingRunRef isolatingRun,
    _SBIsolatingContext *context, _SBIsolatingConsumer consumer)
{
    SBBidiChainRef bidiChain = isolatingRun->bidiChain;
    SBLevelRunRef levelRun;

    /* Iterate over individual level runs of the isolating run. */
    for (levelRun = isolatingRun->baseLevelRun; levelRun; levelRun = levelRun->next) {
        SBBidiLink breakLink = SBBidiChainGetNext(bidiChain, levelRun->lastLink);
        SBBidiLink currentLink = levelRun->firstLink;
        SBBidiLink subsequentLink = levelRun->subsequentLink;

        /* Iterate over each link of the level run. */
        while (currentLink != breakLink) {
            SBBidiLink nextLink = SBBidiChainGetNext(bidiChain, currentLink);
            SBUInteger linkOffset = SBBidiChainGetOffset(bidiChain, currentLink);
            SBUInteger linkLength;
            SBUInteger index;

            if (nextLink != breakLink) {
                linkLength = SBBidiChainGetOffset(bidiChain, nextLink) - linkOffset;
            } else {
                linkLength = SBBidiChainGetOffset(bidiChain, subsequentLink) - linkOffset;
            }

            /* Skip any sequence of BN character types. */
            for (index = 1; index < linkLength; index++) {
                SBCharType charType = SBBidiChainGetType(bidiChain, currentLink + index);
                if (charType == SBCharTypeBN) {
                    linkLength = index;
                    break;
                }
            }

            context->link = currentLink;
            context->length = linkLength;
            consumer(isolatingRun, context);
            
            currentLink = nextLink;
        }
    }
}

static void _SBPrintTypeOperation(SBIsolatingRunRef isolatingRun, _SBIsolatingContext *context)
{
    SBCharType charType = SBBidiChainGetType(isolatingRun->bidiChain, context->link);

    while (context->length--) {
        SB_LOG_CHAR_TYPE(charType);
        SB_LOG_DIVIDER(1);
    }
}

SB_INTERNAL void _SBPrintIsolatingRunTypes(SBIsolatingRunRef isolatingRun)
{
    _SBIsolatingContext context;
    _SBIsolatingRunForEach(isolatingRun, &context, _SBPrintTypeOperation);
}

static void _SBPrintLevelOperation(SBIsolatingRunRef isolatingRun, _SBIsolatingContext *context)
{
    SBLevel charLevel = SBBidiChainGetLevel(isolatingRun->bidiChain, context->link);

    while (context->length--) {
        SB_LOG_LEVEL(charLevel);
        SB_LOG_DIVIDER(1);
    }
}

SB_INTERNAL void _SBPrintIsolatingRunLevels(SBIsolatingRunRef isolatingRun)
{
    _SBIsolatingContext context;
    _SBIsolatingRunForEach(isolatingRun, &context, _SBPrintLevelOperation);
}

typedef struct {
    SBUInteger offset;
    SBUInteger length;
} _SBIsolatingRange;

static void _SBPrintRangeOperation(SBIsolatingRunRef isolatingRun, _SBIsolatingContext *context)
{
    _SBIsolatingRange *range = context->object;
    SBUInteger offset = SBBidiChainGetOffset(isolatingRun->bidiChain, context->link);

    if (range->length == 0) {
        range->offset = offset;
        range->length = context->length;
    } else if (offset == (range->offset + range->length)) {
        range->length += context->length;
    } else {
        SB_LOG_RANGE(range->offset, range->length);
        SB_LOG_DIVIDER(1);

        range->offset = offset;
        range->length = context->length;
    }
}

SB_INTERNAL void _SBPrintIsolatingRunRange(SBIsolatingRunRef isolatingRun)
{
    _SBIsolatingRange range = { 0, 0 };
    _SBIsolatingContext context;
    context.object = &range;

    _SBIsolatingRunForEach(isolatingRun, &context, _SBPrintRangeOperation);
    SB_LOG_RANGE(range.offset, range.length);
    SB_LOG_DIVIDER(1);
}

#endif
