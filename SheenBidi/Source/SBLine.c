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
#include <stdlib.h>

#include "SBAssert.h"
#include "SBCodepointSequence.h"
#include "SBConfig.h"
#include "SBPairingLookup.h"
#include "SBParagraph.h"
#include "SBTypes.h"
#include "SBLine.h"

struct _SBLineSupport;
typedef struct _SBLineSupport _SBLineSupport;
typedef _SBLineSupport *_SBLineSupportRef;

struct _SBLineSupport {
    SBCharType *refTypes;
    SBLevel *fixedLevels;
    SBUInteger runCount;
    SBLevel maxLevel;
};

static SBLevel _SBCopyLevels(SBLevel *destination, SBLevel *source, SBUInteger charCount, SBUInteger *runCount);

static _SBLineSupportRef _SBLineSupportAllocate(SBUInteger charCount);
static void _SBLineSupportInitialize(_SBLineSupportRef support, SBCharType *types, SBLevel *levels, SBUInteger charCount);
static void _SBLineSupportDeallocate(_SBLineSupportRef support);

static SBLineRef _SBLineAllocate(SBUInteger runCount);
static void _SBSetNewLevel(SBLevel *levels, SBUInteger length, SBLevel newLevel);
static void _SBResetLevels(_SBLineSupportRef support, SBLevel baseLevel, SBUInteger charCount);

static SBUInteger _SBInitializeRuns(SBRun *runs, SBLevel *levels, SBUInteger length);
static void _SBReverseRunSequence(SBRun *runs, SBUInteger runCount);
static void _SBReorderRuns(SBRun *runs, SBUInteger runCount, SBLevel maxLevel);

static SBLineRef _SBLineCreate(SBCharType *types, SBLevel *levels, SBUInteger offset, SBUInteger length, SBLevel baseLevel);

static SBLevel _SBCopyLevels(SBLevel *destination, SBLevel *source, SBUInteger charCount, SBUInteger *runCount)
{
    SBLevel lastLevel = SBLevelInvalid;
    SBLevel maxLevel = 0;
    SBUInteger totalRuns = 0;

    while (charCount--) {
        SBLevel level = *(source++);
        *(destination++) = level;

        if (level != lastLevel) {
            ++totalRuns;

            if (level > maxLevel) {
                maxLevel = level;
            }
        }
    }

    *runCount = totalRuns;

    return maxLevel;
}

static _SBLineSupportRef _SBLineSupportAllocate(SBUInteger charCount)
{
    const SBUInteger sizeSupport = sizeof(_SBLineSupport);
    const SBUInteger sizeLevels  = sizeof(SBLevel) * charCount;

    const SBUInteger sizeMemory  = sizeSupport
                                 + sizeLevels;

    SBUInt8 *memory = (SBUInt8 *)malloc(sizeMemory);

    SBUInteger offset = 0;
    _SBLineSupportRef support = (_SBLineSupportRef)(memory + offset);

    offset += sizeSupport;
    support->fixedLevels = (SBLevel *)(memory + offset);

    return support;
}

static void _SBLineSupportInitialize(_SBLineSupportRef support, SBCharType *types, SBLevel *levels, SBUInteger charCount)
{
    SBLevel maxLevel;
    SBUInteger runCount;

    maxLevel = _SBCopyLevels(support->fixedLevels, levels, charCount, &runCount);

    support->refTypes = types;
    support->runCount = runCount;
    support->maxLevel = maxLevel;
}

static void _SBLineSupportDeallocate(_SBLineSupportRef support)
{
    free(support);
}

static SBLineRef _SBLineAllocate(SBUInteger runCount)
{
    const SBUInteger sizeLine = sizeof(SBLine);
    const SBUInteger sizeRuns = sizeof(SBRun) * runCount;

    const SBUInteger sizeMemory = sizeLine
                                + sizeRuns;

    SBUInt8 *memory = (SBUInt8 *)malloc(sizeMemory);
    SBUInteger offset = 0;

    SBLineRef line = (SBLineRef)(memory + offset);
    offset += sizeLine;

    line->fixedRuns = (SBRun *)(memory + offset);

    return line;
}

static void _SBSetNewLevel(SBLevel *levels, SBUInteger length, SBLevel newLevel)
{
    SBUInteger index = length;

    while (index--) {
        levels[index] = newLevel;
    }
}

static void _SBResetLevels(_SBLineSupportRef support, SBLevel baseLevel, SBUInteger charCount)
{
    SBCharType *types = support->refTypes;
    SBLevel *levels = support->fixedLevels;
    SBUInteger index;
    SBUInteger length;
    SBBoolean reset;

    index = charCount;
    length = 0;
    reset = SBTrue;

    while (index--) {
        SBCharType type = types[index];

        switch (type) {
        case SBCharTypeB:
        case SBCharTypeS:
            _SBSetNewLevel(levels + index, length + 1, baseLevel);
            length = 0;
            reset = SBTrue;
            ++support->runCount;
            break;

        case SBCharTypeLRE:
        case SBCharTypeRLE:
        case SBCharTypeLRO:
        case SBCharTypeRLO:
        case SBCharTypePDF:
        case SBCharTypeBN:
            ++length;
            break;

        case SBCharTypeWS:
        case SBCharTypeLRI:
        case SBCharTypeRLI:
        case SBCharTypeFSI:
        case SBCharTypePDI:
            if (reset) {
                _SBSetNewLevel(levels + index, length + 1, baseLevel);
                length = 0;

                ++support->runCount;
            }
            break;

        default:
            length = 0;
            reset = SBFalse;
            break;
        }
    }
}

static SBUInteger _SBInitializeRuns(SBRun *runs, SBLevel *levels, SBUInteger length)
{
    SBUInteger index;
    SBUInteger runCount = 1;

    (*runs).offset = 0;
    (*runs).level = levels[0];

    for (index = 0; index < length; ++index) {
        SBLevel level = levels[index];

        if (level != (*runs).level) {
            (*runs).length = index - (*runs).offset;

            ++runs;
            (*runs).offset = index;
            (*runs).level = level;

            ++runCount;
        }
    }

    (*runs).length = index - (*runs).offset;

    return runCount;
}

static void _SBReverseRunSequence(SBRun *runs, SBUInteger runCount)
{
    SBUInteger halfCount;
    SBUInteger finalIndex;
    SBUInteger index;

    halfCount = runCount / 2;
    finalIndex = runCount - 1;

    for (index = 0; index < halfCount; ++index) {
        SBUInteger tieIndex;
        SBRun tempRun;

        tieIndex = finalIndex - index;

        tempRun = runs[index];
        runs[index] = runs[tieIndex];
        runs[tieIndex] = tempRun;
    }
}

static void _SBReorderRuns(SBRun *runs, SBUInteger runCount, SBLevel maxLevel)
{
    SBLevel newLevel;

    for (newLevel = maxLevel; newLevel; --newLevel) {
        SBUInteger start = runCount;

        while (start--) {
            if (runs[start].level >= newLevel) {
                SBUInteger count = 1;

                for (; start && runs[start - 1].level >= newLevel; --start) {
                    ++count;
                }

                _SBReverseRunSequence(runs+ start, count);
            }
        }
    }
}

static SBLineRef _SBLineCreate(SBCharType *types, SBLevel *levels, SBUInteger offset, SBUInteger length, SBLevel baseLevel)
{
    _SBLineSupportRef support;
    SBLineRef line;

    support = _SBLineSupportAllocate(length);
    _SBLineSupportInitialize(support, types, levels, length);

    _SBResetLevels(support, baseLevel, length);

    line = _SBLineAllocate(support->runCount);
    line->runCount = _SBInitializeRuns(line->fixedRuns, support->fixedLevels, length);
    _SBReorderRuns(line->fixedRuns, line->runCount, support->maxLevel);

    line->offset = offset;
    line->length = length;
    line->_retainCount = 1;

    _SBLineSupportDeallocate(support);

    return line;
}

SB_INTERNAL SBLineRef SBLineCreate(SBParagraphRef paragraph, SBUInteger lineOffset, SBUInteger lineLength)
{
    SBUInteger innerOffset;

    /* Paragraph must NOT be null. */
    SBAssert(paragraph != NULL);
    /* Line range MUST be valid. */
    SBAssert(lineOffset < (lineOffset + lineLength)
             && lineOffset >= paragraph->offset
             && (lineOffset + lineLength) <= (paragraph->offset + paragraph->length));

    innerOffset = lineOffset - paragraph->offset;

    return _SBLineCreate(paragraph->refTypes + innerOffset, paragraph->fixedLevels + innerOffset,
                         lineOffset, lineLength, paragraph->baseLevel);
}

SBUInteger SBLineGetOffset(SBLineRef line)
{
    return line->offset;
}

SBUInteger SBLineGetLength(SBLineRef line)
{
    return line->length;
}

SBLineRef SBLineRetain(SBLineRef line)
{
    if (line) {
        ++line->_retainCount;
    }
    
    return line;
}

void SBLineRelease(SBLineRef line)
{
    if (line && --line->_retainCount == 0) {
        free(line);
    }
}
