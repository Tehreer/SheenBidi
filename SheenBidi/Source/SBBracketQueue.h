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

#ifndef _SB_INTERNAL_BRACKET_QUEUE_H
#define _SB_INTERNAL_BRACKET_QUEUE_H

#include <SBConfig.h>
#include <SBTypes.h>

#include "SBCharType.h"
#include "SBBidiLink.h"

#define _SB_BRACKET_QUEUE_LIST__LENGTH      4
#define _SB_BRACKET_QUEUE_LIST__MAX_INDEX   (_SB_BRACKET_QUEUE_LIST__LENGTH - 1)

struct SBBracketQueue;
struct _SBBracketQueueList;

typedef struct SBBracketQueue SBBracketQueue;
typedef struct _SBBracketQueueList _SBBracketQueueList;

typedef SBBracketQueue *SBBracketQueueRef;
typedef _SBBracketQueueList *_SBBracketQueueListRef;

struct _SBBracketQueueList {
    _SBBracketQueueListRef previous;
    _SBBracketQueueListRef next;

    SBUnichar bracket[_SB_BRACKET_QUEUE_LIST__LENGTH];
    SBBidiLinkRef priorStrongLink[_SB_BRACKET_QUEUE_LIST__LENGTH];
    SBBidiLinkRef openingLink[_SB_BRACKET_QUEUE_LIST__LENGTH];
    SBBidiLinkRef closingLink[_SB_BRACKET_QUEUE_LIST__LENGTH];
    SBCharType strongType[_SB_BRACKET_QUEUE_LIST__LENGTH];
};

struct SBBracketQueue {
    _SBBracketQueueList _firstList;
    _SBBracketQueueListRef _frontList;
    _SBBracketQueueListRef _rearList;
    SBUInteger _frontTop;
    SBUInteger _rearTop;
    SBUInteger count;
    SBBoolean shouldDequeue;
    SBCharType _direction;
};

typedef struct SBBracketQueue *SBBracketQueueRef;

SB_INTERNAL void SBBracketQueueInitialize(SBBracketQueueRef queue);
SB_INTERNAL void SBBracketQueueReset(SBBracketQueueRef queue, SBCharType direction);

SB_INTERNAL void SBBracketQueueEnqueue(SBBracketQueueRef queue, SBBidiLinkRef priorStrongLink, SBBidiLinkRef openingLink, SBUnichar bracket);
SB_INTERNAL void SBBracketQueueDequeue(SBBracketQueueRef queue);

SB_INTERNAL void SBBracketQueueSetStrongType(SBBracketQueueRef queue, SBCharType strongType);
SB_INTERNAL void SBBracketQueueClosePair(SBBracketQueueRef queue, SBBidiLinkRef closingLink, SBUnichar bracket);

SB_INTERNAL SBBoolean SBBracketQueueShouldDequeue(SBBracketQueueRef queue);

SB_INTERNAL SBBidiLinkRef SBBracketQueueGetPriorStrongLink(SBBracketQueueRef queue);
SB_INTERNAL SBBidiLinkRef SBBracketQueueGetOpeningLink(SBBracketQueueRef queue);
SB_INTERNAL SBBidiLinkRef SBBracketQueueGetClosingLink(SBBracketQueueRef queue);
SB_INTERNAL SBCharType SBBracketQueueGetStrongType(SBBracketQueueRef queue);

SB_INTERNAL void SBBracketQueueFinalize(SBBracketQueueRef queue);

#endif
