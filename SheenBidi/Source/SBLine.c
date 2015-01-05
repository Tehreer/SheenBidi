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

#include <stdlib.h>

#include <SBConfig.h>
#include <SBTypes.h>

#include "SBPairingLookup.h"
#include "SBParagraph.h"

#include "SBLine.h"

struct __SBLineSupport;
typedef struct __SBLineSupport __SBLineSupport;
typedef __SBLineSupport *__SBLineSupportRef;

struct __SBLineSupport {
    _SBCharType *refTypes;
    SBLevel *fixedLevels;
    SBUInteger runCount;
    SBLevel maxLevel;
};

static SBLevel __SBCopyLevels(SBLevel *destination, SBLevel *source, SBUInteger charCount, SBUInteger *runCount) {
    SBLevel lastLevel = SBInvalidLevel;
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

static __SBLineSupportRef __SBLineSupportAllocate(SBUInteger charCount) {
    const SBUInteger sizeSupport = sizeof(__SBLineSupport);
    const SBUInteger sizeLevels  = sizeof(SBLevel) * charCount;

    const SBUInteger sizeMemory  = sizeSupport
                                 + sizeLevels;

    SBUInt8 *memory = (SBUInt8 *)malloc(sizeMemory);

    SBUInteger offset = 0;
    __SBLineSupportRef support = (__SBLineSupportRef)(memory + offset);

    offset += sizeSupport;
    support->fixedLevels = (SBLevel *)(memory + offset);

    return support;
}

static void __SBLineSupportInitialize(__SBLineSupportRef support, _SBCharType *types, SBLevel *levels, SBUInteger charCount) {
    SBLevel maxLevel;
    SBUInteger runCount;

    maxLevel = __SBCopyLevels(support->fixedLevels, levels, charCount, &runCount);

    support->refTypes = types;
    support->runCount = runCount;
    support->maxLevel = maxLevel;
}

static void __SBLineSupportDeallocate(__SBLineSupportRef support) {
    free(support);
}

static SBLineRef __SBLineAllocate(SBUInteger runCount) {
    const SBUInteger sizeLine = sizeof(SBLine);
    const SBUInteger sizeRuns = sizeof(_SBRun) * runCount;

    const SBUInteger sizeMemory = sizeLine
                                + sizeRuns;

    SBUInt8 *memory = (SBUInt8 *)malloc(sizeMemory);
    SBUInteger offset = 0;

    SBLineRef line = (SBLineRef)(memory + offset);
    offset += sizeLine;

    line->fixedRuns = (_SBRun *)(memory + offset);

    return line;
}

static void __SBSetNewLevel(SBLevel *levels, SBUInteger length, SBLevel newLevel) {
    SBUInteger index = length;

    while (index--) {
        levels[index] = newLevel;
    }
}

static void __SBResetLevels(__SBLineSupportRef support, SBLevel baseLevel, SBUInteger charCount) {
    _SBCharType *types = support->refTypes;
    SBLevel *levels = support->fixedLevels;

    SBUInteger index;
    SBUInteger length;
    SBBoolean reset;

    index = charCount;
    length = 0;
    reset = SBTrue;

    while (index--) {
        _SBCharType type = types[index];

        switch (type) {
        case _SB_CHAR_TYPE__B:
        case _SB_CHAR_TYPE__S:
            __SBSetNewLevel(levels + index, length + 1, baseLevel);
            length = 0;
            reset = SBTrue;

            ++support->runCount;
            break;

        case _SB_CHAR_TYPE__BN_EQUIVALENT_CASE:
            ++length;
            break;

        case _SB_CHAR_TYPE__WHITESPACE_OR_ISOLATE_CASE:
            if (reset) {
                __SBSetNewLevel(levels + index, length + 1, baseLevel);
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

static SBUInteger __SBInitializeRuns(_SBRun *runs, SBLevel *levels, SBUInteger length) {
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

static void __SBReverseRunSequence(_SBRun *runs, SBUInteger runCount) {
    SBUInteger halfCount;
    SBUInteger finalIndex;
    SBUInteger index;

    halfCount = runCount / 2;
    finalIndex = runCount - 1;

    for (index = 0; index < halfCount; ++index) {
        SBUInteger tieIndex;
        _SBRun tempRun;

        tieIndex = finalIndex - index;

        tempRun = runs[index];
        runs[index] = runs[tieIndex];
        runs[tieIndex] = tempRun;
    }
}

static void __SBReorderRuns(_SBRun *runs, SBUInteger runCount, SBLevel maxLevel) {
    SBLevel newLevel;

    for (newLevel = maxLevel; newLevel; --newLevel) {
        SBUInteger start = runCount;

        while (start--) {
            if (runs[start].level >= newLevel) {
                SBUInteger count = 1;

                for (; start && runs[start - 1].level >= newLevel; --start) {
                    ++count;
                }

                __SBReverseRunSequence(runs+ start, count);
            }
        }
    }
}

static SBLineRef __SBLineCreate(_SBCharType *types, SBLevel *levels, SBUInteger offset, SBUInteger length, SBLevel baseLevel, SBLineOptions options) {
    __SBLineSupportRef support;
    SBLineRef line;

    support = __SBLineSupportAllocate(length);
    __SBLineSupportInitialize(support, types, levels, length);

    __SBResetLevels(support, baseLevel, length);

    line = __SBLineAllocate(support->runCount);
    line->runCount = __SBInitializeRuns(line->fixedRuns, support->fixedLevels, length);
    __SBReorderRuns(line->fixedRuns, line->runCount, support->maxLevel);

    line->offset = offset;
    line->length = length;
    line->_retainCount = 1;

    __SBLineSupportDeallocate(support);

    return line;
}

SBLineRef SBLineCreateWithUnicodeCharacters(SBUnichar *characters, SBUInteger length, SBBaseDirection direction, SBLineOptions options) {
    SBParagraphRef paragraph;
    SBLineRef line;

    paragraph = SBParagraphCreateWithUnicodeCharacters(characters, length, direction, SBParagraphOptionsNone);
    line = SBLineCreateWithParagraph(paragraph, 0, length, options);

    SBParagraphRelease(paragraph);

    return line;
}

SBLineRef SBLineCreateWithParagraph(SBParagraphRef paragraph, SBUInteger offset, SBUInteger length, SBLineOptions options) {
    return __SBLineCreate(paragraph->fixedTypes,
                          paragraph->fixedLevels,
                          offset, length,
                          paragraph->baseLevel,
                          options);
}

SBUInteger SBLineGetOffset(SBLineRef line) {
    return line->offset;
}

SBUInteger SBLineGetLength(SBLineRef line) {
    return line->length;
}

SBLineRef SBLineRetain(SBLineRef line) {
    if (line) {
        ++line->_retainCount;
    }
    
    return line;
}

void SBLineRelease(SBLineRef line) {
    if (line && --line->_retainCount == 0) {
        free(line);
    }
}
