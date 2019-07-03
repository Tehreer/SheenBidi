/*
 * Copyright (C) 2014-2019 Muhammad Tayyab Akram
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

#include "PairingLookup.h"
#include "SBAlgorithm.h"
#include "SBAssert.h"
#include "SBBase.h"
#include "SBCodepointSequence.h"
#include "SBParagraph.h"
#include "SBRun.h"
#include "SBLine.h"

typedef struct _LineSupport {
    const SBBidiType *refTypes;
    SBLevel *fixedLevels;
    SBUInteger runCount;
    SBLevel maxLevel;
} LineSupport, *LineSupportRef;

static SBLevel CopyLevels(SBLevel *destination,
    const SBLevel *source, SBUInteger charCount, SBUInteger *runCount)
{
    SBLevel lastLevel = SBLevelInvalid;
    SBLevel maxLevel = 0;
    SBUInteger totalRuns = 0;

    while (charCount--) {
        SBLevel level = *(source++);
        *(destination++) = level;

        if (level != lastLevel) {
            totalRuns += 1;

            if (level > maxLevel) {
                maxLevel = level;
            }
        }
    }

    *runCount = totalRuns;

    return maxLevel;
}

static LineSupportRef LineSupportAllocate(SBUInteger charCount)
{
    const SBUInteger sizeSupport = sizeof(LineSupport);
    const SBUInteger sizeLevels  = sizeof(SBLevel) * charCount;
    const SBUInteger sizeMemory  = sizeSupport + sizeLevels;

    const SBUInteger offsetSupport = 0;
    const SBUInteger offsetLevels  = offsetSupport + sizeSupport;

    SBUInt8 *memory = (SBUInt8 *)malloc(sizeMemory);
    LineSupportRef support = (LineSupportRef)(memory + offsetSupport);
    SBLevel *levels = (SBLevel *)(memory + offsetLevels);

    support->fixedLevels = levels;

    return support;
}

static void LineSupportInitialize(LineSupportRef support,
    const SBBidiType *types, SBLevel *levels, SBUInteger charCount)
{
    SBLevel maxLevel;
    SBUInteger runCount;

    maxLevel = CopyLevels(support->fixedLevels, levels, charCount, &runCount);

    support->refTypes = types;
    support->runCount = runCount;
    support->maxLevel = maxLevel;
}

static void LineSupportDeallocate(LineSupportRef support)
{
    free(support);
}

static SBLineRef LineAllocate(SBUInteger runCount)
{
    const SBUInteger sizeLine   = sizeof(SBLine);
    const SBUInteger sizeRuns   = sizeof(SBRun) * runCount;
    const SBUInteger sizeMemory = sizeLine + sizeRuns;

    const SBUInteger offsetLine = 0;
    const SBUInteger offsetRuns = offsetLine + sizeLine;

    SBUInt8 *memory = (SBUInt8 *)malloc(sizeMemory);
    SBLineRef line = (SBLineRef)(memory + offsetLine);
    SBRun *runs = (SBRun *)(memory + offsetRuns);

    line->fixedRuns = runs;

    return line;
}

static void SetNewLevel(SBLevel *levels, SBUInteger length, SBLevel newLevel)
{
    SBUInteger index = length;

    while (index--) {
        levels[index] = newLevel;
    }
}

static void ResetLevels(LineSupportRef support, SBLevel baseLevel, SBUInteger charCount)
{
    const SBBidiType *types = support->refTypes;
    SBLevel *levels = support->fixedLevels;
    SBUInteger index;
    SBUInteger length;
    SBBoolean reset;

    index = charCount;
    length = 0;
    reset = SBTrue;

    while (index--) {
        SBBidiType type = types[index];

        switch (type) {
        case SBBidiTypeB:
        case SBBidiTypeS:
            SetNewLevel(levels + index, length + 1, baseLevel);
            length = 0;
            reset = SBTrue;
            support->runCount += 1;
            break;

        case SBBidiTypeLRE:
        case SBBidiTypeRLE:
        case SBBidiTypeLRO:
        case SBBidiTypeRLO:
        case SBBidiTypePDF:
        case SBBidiTypeBN:
            length += 1;
            break;

        case SBBidiTypeWS:
        case SBBidiTypeLRI:
        case SBBidiTypeRLI:
        case SBBidiTypeFSI:
        case SBBidiTypePDI:
            if (reset) {
                SetNewLevel(levels + index, length + 1, baseLevel);
                length = 0;

                support->runCount += 1;
            }
            break;

        default:
            length = 0;
            reset = SBFalse;
            break;
        }
    }
}

static SBUInteger InitializeRuns(SBRun *runs,
    const SBLevel *levels, SBUInteger length, SBUInteger lineOffset)
{
    SBUInteger index;
    SBUInteger runCount = 1;

    (*runs).offset = lineOffset;
    (*runs).level = levels[0];

    for (index = 0; index < length; index++) {
        SBLevel level = levels[index];

        if (level != (*runs).level) {
            (*runs).length = index + lineOffset - (*runs).offset;

            ++runs;
            (*runs).offset = lineOffset + index;
            (*runs).level = level;

            runCount += 1;
        }
    }

    (*runs).length = index + lineOffset - (*runs).offset;

    return runCount;
}

static void ReverseRunSequence(SBRun *runs, SBUInteger runCount)
{
    SBUInteger halfCount = runCount / 2;
    SBUInteger finalIndex = runCount - 1;
    SBUInteger index;

    for (index = 0; index < halfCount; index++) {
        SBUInteger tieIndex;
        SBRun tempRun;

        tieIndex = finalIndex - index;

        tempRun = runs[index];
        runs[index] = runs[tieIndex];
        runs[tieIndex] = tempRun;
    }
}

static void ReorderRuns(SBRun *runs, SBUInteger runCount, SBLevel maxLevel)
{
    SBLevel newLevel;

    for (newLevel = maxLevel; newLevel; newLevel--) {
        SBUInteger start = runCount;

        while (start--) {
            if (runs[start].level >= newLevel) {
                SBUInteger count = 1;

                for (; start && runs[start - 1].level >= newLevel; start--) {
                    count += 1;
                }

                ReverseRunSequence(runs + start, count);
            }
        }
    }
}

static SBLineRef LineCreate(const SBCodepointSequence *codepointSequence,
    const SBBidiType *types, SBLevel *levels, SBUInteger offset, SBUInteger length, SBLevel baseLevel)
{
    LineSupportRef support;
    SBLineRef line;

    support = LineSupportAllocate(length);
    LineSupportInitialize(support, types, levels, length);

    ResetLevels(support, baseLevel, length);

    line = LineAllocate(support->runCount);
    line->runCount = InitializeRuns(line->fixedRuns, support->fixedLevels, length, offset);
    ReorderRuns(line->fixedRuns, line->runCount, support->maxLevel);

    line->codepointSequence = *codepointSequence;
    line->offset = offset;
    line->length = length;
    line->_retainCount = 1;

    LineSupportDeallocate(support);

    return line;
}

SB_INTERNAL SBLineRef SBLineCreate(SBParagraphRef paragraph,
    SBUInteger lineOffset, SBUInteger lineLength)
{
    SBUInteger innerOffset;

    /* Paragraph must NOT be null. */
    SBAssert(paragraph != NULL);
    /* Line range MUST be valid. */
    SBAssert(lineOffset < (lineOffset + lineLength)
             && lineOffset >= paragraph->offset
             && (lineOffset + lineLength) <= (paragraph->offset + paragraph->length));

    innerOffset = lineOffset - paragraph->offset;

    return LineCreate(&paragraph->algorithm->codepointSequence,
                      paragraph->refTypes + innerOffset, paragraph->fixedLevels + innerOffset,
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

SBUInteger SBLineGetRunCount(SBLineRef line)
{
    return line->runCount;
}

const SBRun *SBLineGetRunsPtr(SBLineRef line)
{
    return line->fixedRuns;
}

SBLineRef SBLineRetain(SBLineRef line)
{
    if (line) {
        line->_retainCount += 1;
    }
    
    return line;
}

void SBLineRelease(SBLineRef line)
{
    if (line && --line->_retainCount == 0) {
        free(line);
    }
}
