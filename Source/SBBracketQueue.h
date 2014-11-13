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

#ifndef _SB_BRACKET_QUEUE_H
#define _SB_BRACKET_QUEUE_H

#include <SBConfig.h>
#include <SBTypes.h>

#include "SBCharType.h"
#include "SBRunLink.h"

#define __SB_BRACKET_QUEUE_LIST__LENGTH     4
#define __SB_BRACKET_QUEUE_LIST__MAX_INDEX  (__SB_BRACKET_QUEUE_LIST__LENGTH - 1)

struct _SBBracketQueue;
struct __SBBracketQueueList;

typedef struct _SBBracketQueue _SBBracketQueue;
typedef struct __SBBracketQueueList __SBBracketQueueList;

typedef _SBBracketQueue *_SBBracketQueueRef;
typedef __SBBracketQueueList *__SBBracketQueueListRef;

struct __SBBracketQueueList {
    __SBBracketQueueListRef previous;
    __SBBracketQueueListRef next;

    SBUnichar bracket[__SB_BRACKET_QUEUE_LIST__LENGTH];
    _SBRunLinkRef priorStrongLink[__SB_BRACKET_QUEUE_LIST__LENGTH];
    _SBRunLinkRef openingLink[__SB_BRACKET_QUEUE_LIST__LENGTH];
    _SBRunLinkRef closingLink[__SB_BRACKET_QUEUE_LIST__LENGTH];
    _SBCharType strongType[__SB_BRACKET_QUEUE_LIST__LENGTH];
};

struct _SBBracketQueue {
    __SBBracketQueueList _firstList;
    __SBBracketQueueListRef _frontList;
    __SBBracketQueueListRef _rearList;
    SBUInteger _frontTop;
    SBUInteger _rearTop;
    SBUInteger count;
    SBBoolean shouldDequeue;
    _SBCharType _direction;
};

typedef struct _SBBracketQueue *_SBBracketQueueRef;

SB_INTERNAL void _SBBracketQueueInitialize(_SBBracketQueueRef queue);
SB_INTERNAL void _SBBracketQueueReset(_SBBracketQueueRef queue, _SBCharType direction);

SB_INTERNAL void _SBBracketQueueEnqueue(_SBBracketQueueRef queue, _SBRunLinkRef priorStrongLink, _SBRunLinkRef openingLink, SBUnichar bracket);
SB_INTERNAL void _SBBracketQueueDequeue(_SBBracketQueueRef queue);

SB_INTERNAL void _SBBracketQueueSetStrongType(_SBBracketQueueRef queue, _SBCharType strongType);
SB_INTERNAL void _SBBracketQueueClosePair(_SBBracketQueueRef queue, _SBRunLinkRef closingLink, SBUnichar bracket);

SB_INTERNAL SBBoolean _SBBracketQueueShouldDequeue(_SBBracketQueueRef queue);

SB_INTERNAL _SBRunLinkRef _SBBracketQueueGetPriorStrongLink(_SBBracketQueueRef queue);
SB_INTERNAL _SBRunLinkRef _SBBracketQueueGetOpeningLink(_SBBracketQueueRef queue);
SB_INTERNAL _SBRunLinkRef _SBBracketQueueGetClosingLink(_SBBracketQueueRef queue);
SB_INTERNAL _SBCharType _SBBracketQueueGetStrongType(_SBBracketQueueRef queue);

SB_INTERNAL void _SBBracketQueueInvalidate(_SBBracketQueueRef queue);

#endif
