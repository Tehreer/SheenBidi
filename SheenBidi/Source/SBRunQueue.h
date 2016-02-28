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

#ifndef _SB_INTERNAL_RUN_QUEUE_H
#define _SB_INTERNAL_RUN_QUEUE_H

#include <SBConfig.h>

#include "SBLevelRun.h"
#include "SBTypes.h"

#define _SB_RUN_QUEUE_LIST__LENGTH     8
#define _SB_RUN_QUEUE_LIST__MAX_INDEX  (_SB_RUN_QUEUE_LIST__LENGTH - 1)

struct _SBRunQueueList;
typedef struct _SBRunQueueList _SBRunQueueList;
typedef _SBRunQueueList *_SBRunQueueListRef;

struct _SBRunQueueList {
    _SBRunQueueListRef previous;        /**< Reference to the previous list of queue elements */
    _SBRunQueueListRef next;            /**< Reference to the next list of queue elements */

    SBLevelRun levelRuns[_SB_RUN_QUEUE_LIST__LENGTH];
};

typedef struct _SBRunQueue {
    _SBRunQueueList _firstList;         /**< First list of elements, which is part of the queue */
    _SBRunQueueListRef _frontList;      /**< The list containing front element of the queue */
    _SBRunQueueListRef _rearList;       /**< The list containing rear element of the queue */
    _SBRunQueueListRef _partialList;    /**< The list containing latest partial isolating run */
    SBUInteger _frontTop;               /**< Index of front element in front list */
    SBUInteger _rearTop;                /**< Index of rear element in rear list */
    SBUInteger _partialTop;             /**< Index of partial run in partial list */
    SBLevelRunRef peek;                 /**< Peek element of the queue */
    SBUInteger count;                   /**< Number of elements the queue contains */
    SBBoolean shouldDequeue;
} SBRunQueue, *SBRunQueueRef;

SB_INTERNAL void SBRunQueueInitialize(SBRunQueueRef queue);

SB_INTERNAL void SBRunQueueEnqueue(SBRunQueueRef queue, SBLevelRun levelRun);
SB_INTERNAL void SBRunQueueDequeue(SBRunQueueRef queue);

SB_INTERNAL void SBRunQueueFinalize(SBRunQueueRef queue);

#endif
