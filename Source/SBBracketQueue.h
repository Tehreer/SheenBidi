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

#ifndef _SB_INTERNAL_BRACKET_QUEUE_H
#define _SB_INTERNAL_BRACKET_QUEUE_H

#include <SBConfig.h>

#include "SBBase.h"
#include "SBBidiLink.h"
#include "SBCharType.h"

#define _SB_BRACKET_QUEUE_LIST__LENGTH      8
#define _SB_BRACKET_QUEUE_LIST__MAX_INDEX   (_SB_BRACKET_QUEUE_LIST__LENGTH - 1)

typedef struct _SBBracketQueueList *_SBBracketQueueListRef;

typedef struct _SBBracketQueueList {
    _SBBracketQueueListRef previous;
    _SBBracketQueueListRef next;

    SBCodepoint bracket[_SB_BRACKET_QUEUE_LIST__LENGTH];
    SBBidiLinkRef priorStrongLink[_SB_BRACKET_QUEUE_LIST__LENGTH];
    SBBidiLinkRef openingLink[_SB_BRACKET_QUEUE_LIST__LENGTH];
    SBBidiLinkRef closingLink[_SB_BRACKET_QUEUE_LIST__LENGTH];
    SBCharType strongType[_SB_BRACKET_QUEUE_LIST__LENGTH];
} _SBBracketQueueList;

typedef struct _SBBracketQueue {
    _SBBracketQueueList _firstList;
    _SBBracketQueueListRef _frontList;
    _SBBracketQueueListRef _rearList;
    SBUInteger _frontTop;
    SBUInteger _rearTop;
    SBUInteger count;
    SBBoolean shouldDequeue;
    SBCharType _direction;
} SBBracketQueue, *SBBracketQueueRef;

#define SBBracketQueueGetMaxCapacity()      63

SB_INTERNAL void SBBracketQueueInitialize(SBBracketQueueRef queue);
SB_INTERNAL void SBBracketQueueReset(SBBracketQueueRef queue, SBCharType direction);

SB_INTERNAL void SBBracketQueueEnqueue(SBBracketQueueRef queue, SBBidiLinkRef priorStrongLink, SBBidiLinkRef openingLink, SBCodepoint bracket);
SB_INTERNAL void SBBracketQueueDequeue(SBBracketQueueRef queue);

SB_INTERNAL void SBBracketQueueSetStrongType(SBBracketQueueRef queue, SBCharType strongType);
SB_INTERNAL void SBBracketQueueClosePair(SBBracketQueueRef queue, SBBidiLinkRef closingLink, SBCodepoint bracket);

SB_INTERNAL SBBoolean SBBracketQueueShouldDequeue(SBBracketQueueRef queue);

SB_INTERNAL SBBidiLinkRef SBBracketQueueGetPriorStrongLink(SBBracketQueueRef queue);
SB_INTERNAL SBBidiLinkRef SBBracketQueueGetOpeningLink(SBBracketQueueRef queue);
SB_INTERNAL SBBidiLinkRef SBBracketQueueGetClosingLink(SBBracketQueueRef queue);
SB_INTERNAL SBCharType SBBracketQueueGetStrongType(SBBracketQueueRef queue);

SB_INTERNAL void SBBracketQueueFinalize(SBBracketQueueRef queue);

#endif
