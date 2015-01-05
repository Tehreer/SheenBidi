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

#ifndef _SB_RUN_QUEUE_H
#define _SB_RUN_QUEUE_H

#include <SBConfig.h>
#include <SBTypes.h>

#include "SBLevelRun.h"

#define __SB_RUN_QUEUE_LIST__LENGTH     8
#define __SB_RUN_QUEUE_LIST__MAX_INDEX  (__SB_RUN_QUEUE_LIST__LENGTH - 1)

struct __SBRunQueue;
struct __SBRunQueueList;

typedef struct __SBRunQueue _SBRunQueue;
typedef struct __SBRunQueueList __SBRunQueueList;

typedef _SBRunQueue *_SBRunQueueRef;
typedef __SBRunQueueList *__SBRunQueueListRef;

struct __SBRunQueueList {
    __SBRunQueueListRef previous;       /**< Reference to the previous list of queue elements */
    __SBRunQueueListRef next;           /**< Reference to the next list of queue elements */

    _SBLevelRun levelRuns[__SB_RUN_QUEUE_LIST__LENGTH];
};

struct __SBRunQueue {
    __SBRunQueueList _firstList;        /**< First list of elements, which is part of the queue */
    __SBRunQueueListRef _frontList;     /**< The list containing front element of the queue */
    __SBRunQueueListRef _rearList;      /**< The list containing rear element of the queue */
    __SBRunQueueListRef _partialList;   /**< The list containing latest partial isolating run */
    SBUInteger _frontTop;               /**< Index of front element in front list */
    SBUInteger _rearTop;                /**< Index of rear element in rear list */
    SBUInteger _partialTop;             /**< Index of partial run in partial list */
    _SBLevelRunRef peek;                /**< Peek element of the queue */
    SBUInteger count;                   /**< Number of elements the queue contains */
    SBBoolean shouldDequeue;
};

SB_INTERNAL void _SBRunQueueInitialize(_SBRunQueueRef queue);

SB_INTERNAL void _SBRunQueueEnqueue(_SBRunQueueRef queue, _SBLevelRun levelRun);
SB_INTERNAL void _SBRunQueueDequeue(_SBRunQueueRef queue);

SB_INTERNAL void _SBRunQueueInvalidate(_SBRunQueueRef queue);

#endif
