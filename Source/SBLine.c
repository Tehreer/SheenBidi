/*
 * Copyright (C) 2014-2025 Muhammad Tayyab Akram
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

#include <SheenBidi/SBConfig.h>
#include <SheenBidi/SBRun.h>

#include "Object.h"
#include "PairingLookup.h"
#include "SBAlgorithm.h"
#include "SBAssert.h"
#include "SBBase.h"
#include "SBCodepointSequence.h"
#include "SBLine.h"
#include "SBParagraph.h"

typedef struct _LineContext {
    Object object;
    const SBBidiType *refTypes;
    SBLevel *fixedLevels;
    SBUInteger runCount;
    SBLevel maxLevel;
} LineContext, *LineContextRef;

static SBLevel CopyLevels(SBLevel *destination,
    const SBLevel *source, SBUInteger length, SBUInteger *runCount)
{
    SBLevel lastLevel = SBLevelInvalid;
    SBLevel maxLevel = 0;
    SBUInteger totalRuns = 0;

    while (length--) {
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

#define LINE_CONTEXT 0
#define LEVELS       1
#define COUNT        2

static LineContextRef CreateLineContext(const SBBidiType *types, const SBLevel *levels, SBUInteger length)
{
    void *pointers[COUNT] = { NULL };
    SBUInteger sizes[COUNT];

    sizes[LINE_CONTEXT] = sizeof(LineContext);
    sizes[LEVELS]       = sizeof(SBLevel) * length;

    if (ObjectCreate(sizes, COUNT, pointers)) {
        LineContextRef context = pointers[LINE_CONTEXT];
        SBLevel *fixedLevels = pointers[LEVELS];

        context->refTypes = types;
        context->fixedLevels = fixedLevels;
        context->maxLevel = CopyLevels(fixedLevels, levels, length, &context->runCount);
    }

    return pointers[LINE_CONTEXT];
}

#undef LINE_CONTEXT
#undef LEVELS
#undef COUNT

static void DisposeLineContext(LineContextRef context)
{
    ObjectDispose(&context->object);
}

#define LINE  0
#define RUNS  1
#define COUNT 2

static SBLineRef AllocateLine(SBUInteger runCount)
{
    void *pointers[COUNT] = { NULL };
    SBUInteger sizes[COUNT];

    sizes[LINE] = sizeof(SBLine);
    sizes[RUNS] = sizeof(SBRun) * runCount;

    if (ObjectCreate(sizes, COUNT, pointers)) {
        SBLineRef line = pointers[LINE];
        SBRun *runs = pointers[RUNS];

        line->fixedRuns = runs;
    }

    return pointers[LINE];
}

#undef LINE
#undef RUNS
#undef COUNT

static void SetNewLevel(SBLevel *levels, SBUInteger length, SBLevel newLevel)
{
    SBUInteger index = length;

    while (index--) {
        levels[index] = newLevel;
    }
}

static void ResetLevels(LineContextRef context, SBLevel baseLevel, SBUInteger charCount)
{
    const SBBidiType *types = context->refTypes;
    SBLevel *levels = context->fixedLevels;
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
            context->runCount += 1;
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

                context->runCount += 1;
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

SB_INTERNAL SBLineRef SBLineCreate(SBParagraphRef paragraph,
    SBUInteger lineOffset, SBUInteger lineLength)
{
    SBUInteger innerOffset = lineOffset - paragraph->offset;
    const SBBidiType *refTypes = paragraph->refTypes + innerOffset;
    const SBLevel *refLevels = paragraph->fixedLevels + innerOffset;
    LineContextRef context;
    SBLineRef line;

    /* Line range MUST be valid. */
    SBAssert(lineOffset < (lineOffset + lineLength)
             && lineOffset >= paragraph->offset
             && (lineOffset + lineLength) <= (paragraph->offset + paragraph->length));

    context = CreateLineContext(refTypes, refLevels, lineLength);

    if (context) {
        ResetLevels(context, paragraph->baseLevel, lineLength);

        line = AllocateLine(context->runCount);

        if (line) {
            line->runCount = InitializeRuns(line->fixedRuns, context->fixedLevels, lineLength, lineOffset);
            ReorderRuns(line->fixedRuns, line->runCount, context->maxLevel);

            line->codepointSequence = paragraph->algorithm->codepointSequence;
            line->offset = lineOffset;
            line->length = lineLength;
            line->retainCount = 1;
        }

        DisposeLineContext(context);

        return line;
    }

    return NULL;
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
        line->retainCount += 1;
    }
    
    return line;
}

void SBLineRelease(SBLineRef line)
{
    if (line && --line->retainCount == 0) {
        ObjectDispose(&line->_object);
    }
}
