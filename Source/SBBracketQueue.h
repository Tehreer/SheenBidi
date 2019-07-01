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

#ifndef _SB_INTERNAL_BRACKET_QUEUE_H
#define _SB_INTERNAL_BRACKET_QUEUE_H

#include <SBConfig.h>

#include "SBBase.h"
#include "SBBidiChain.h"

#define BracketQueueList_Length         8
#define BracketQueueList_MaxIndex       (BracketQueueList_Length - 1)

typedef struct _BracketQueueList {
    SBCodepoint bracket[BracketQueueList_Length];
    SBBidiLink priorStrongLink[BracketQueueList_Length];
    SBBidiLink openingLink[BracketQueueList_Length];
    SBBidiLink closingLink[BracketQueueList_Length];
    SBBidiType strongType[BracketQueueList_Length];

    struct _BracketQueueList *previous;
    struct _BracketQueueList *next;
} BracketQueueList, *BracketQueueListRef;

typedef struct _SBBracketQueue {
    BracketQueueList _firstList;
    BracketQueueListRef _frontList;
    BracketQueueListRef _rearList;
    SBInteger _frontTop;
    SBInteger _rearTop;
    SBUInteger count;
    SBBoolean shouldDequeue;
    SBBidiType _direction;
} SBBracketQueue, *SBBracketQueueRef;

#define SBBracketQueueGetMaxCapacity()      63

SB_INTERNAL void SBBracketQueueInitialize(SBBracketQueueRef queue);
SB_INTERNAL void SBBracketQueueReset(SBBracketQueueRef queue, SBBidiType direction);

SB_INTERNAL void SBBracketQueueEnqueue(SBBracketQueueRef queue,
    SBBidiLink priorStrongLink, SBBidiLink openingLink, SBCodepoint bracket);
SB_INTERNAL void SBBracketQueueDequeue(SBBracketQueueRef queue);

SB_INTERNAL void SBBracketQueueSetStrongType(SBBracketQueueRef queue, SBBidiType strongType);
SB_INTERNAL void SBBracketQueueClosePair(SBBracketQueueRef queue,
    SBBidiLink closingLink, SBCodepoint bracket);

SB_INTERNAL SBBoolean SBBracketQueueShouldDequeue(SBBracketQueueRef queue);

SB_INTERNAL SBBidiLink SBBracketQueueGetPriorStrongLink(SBBracketQueueRef queue);
SB_INTERNAL SBBidiLink SBBracketQueueGetOpeningLink(SBBracketQueueRef queue);
SB_INTERNAL SBBidiLink SBBracketQueueGetClosingLink(SBBracketQueueRef queue);
SB_INTERNAL SBBidiType SBBracketQueueGetStrongType(SBBracketQueueRef queue);

SB_INTERNAL void SBBracketQueueFinalize(SBBracketQueueRef queue);

#endif
