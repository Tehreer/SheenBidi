/*
 * Copyright (C) 2014-2018 Muhammad Tayyab Akram
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

#include "SBBase.h"
#include "SBLevelRun.h"

#define _SBRunQueueList_Length      8
#define _SBRunQueueList_MaxIndex    (_SBRunQueueList_Length - 1)

typedef struct _SBRunQueueList {
    SBLevelRun elements[_SBRunQueueList_Length];

    struct _SBRunQueueList *previous;   /**< Reference to the previous list of queue elements */
    struct _SBRunQueueList *next;       /**< Reference to the next list of queue elements */
} _SBRunQueueList, *_SBRunQueueListRef;

typedef struct _SBRunQueue {
    _SBRunQueueList _firstList;         /**< First list of elements, which is part of the queue */
    _SBRunQueueListRef _frontList;      /**< The list containing front element of the queue */
    _SBRunQueueListRef _rearList;       /**< The list containing rear element of the queue */
    _SBRunQueueListRef _partialList;    /**< The list containing latest partial isolating run */
    SBInteger _frontTop;                /**< Index of front element in front list */
    SBInteger _rearTop;                 /**< Index of rear element in rear list */
    SBInteger _partialTop;              /**< Index of partial run in partial list */
    SBLevelRunRef peek;                 /**< Peek element of the queue */
    SBUInteger count;                   /**< Number of elements the queue contains */
    SBBoolean shouldDequeue;
} SBRunQueue, *SBRunQueueRef;

SB_INTERNAL void SBRunQueueInitialize(SBRunQueueRef queue);

SB_INTERNAL void SBRunQueueEnqueue(SBRunQueueRef queue, const SBLevelRunRef levelRun);
SB_INTERNAL void SBRunQueueDequeue(SBRunQueueRef queue);

SB_INTERNAL void SBRunQueueFinalize(SBRunQueueRef queue);

#endif
