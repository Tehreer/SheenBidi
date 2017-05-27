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

#include <SBConfig.h>

#include <stddef.h>
#include <stdlib.h>

#include "SBAssert.h"
#include "SBBase.h"
#include "SBLevelRun.h"
#include "SBRunQueue.h"

static void _SBRunQueueFindPreviousPartialRun(SBRunQueueRef queue)
{
    _SBRunQueueListRef list = queue->_partialList;
    SBUInteger top = queue->_partialTop;

    do {
        SBUInteger limit = (list == queue->_frontList ? queue->_frontTop : 0);

        do {
            SBLevelRunRef levelRun = &list->levelRuns[top];
            if (SBRunKindIsPartialIsolate(levelRun->kind)) {
                queue->_partialList = list;
                queue->_partialTop = top;
                return;
            }
        } while (top-- > limit);

        list = list->previous;
        top = _SBRunQueueList_MaxIndex;
    } while (list);

    queue->_partialList = NULL;
    queue->_partialTop = SBInvalidIndex;
    queue->shouldDequeue = SBFalse;
}

SB_INTERNAL void SBRunQueueInitialize(SBRunQueueRef queue)
{
    /* Initialize first list. */
    queue->_firstList.previous = NULL;
    queue->_firstList.next = NULL;

    /* Initialize front and rear lists with first list. */
    queue->_frontList = &queue->_firstList;
    queue->_rearList = &queue->_firstList;
    queue->_partialList = NULL;

    /* Initialize list indexes. */
    queue->_frontTop = 0;
    queue->_rearTop = SBInvalidIndex;
    queue->_partialTop = SBInvalidIndex;

    /* Initialize rest of the elements. */
    queue->count = 0;
    queue->peek = &queue->_frontList->levelRuns[queue->_frontTop];
    queue->shouldDequeue = SBFalse;
}

SB_INTERNAL void SBRunQueueEnqueue(SBRunQueueRef queue, SBLevelRun levelRun)
{
    SBLevelRunRef current;
    _SBRunQueueListRef list;
    SBUInteger top;

    if (queue->_rearTop != _SBRunQueueList_MaxIndex) {
        list = queue->_rearList;
        top = ++queue->_rearTop;
    } else {
        _SBRunQueueListRef rearList;

        rearList = queue->_rearList;
        list = rearList->next;

        if (!list) {
            list = malloc(sizeof(_SBRunQueueList));
            list->previous = rearList;
            list->next = NULL;

            rearList->next = list;
        }

        queue->_rearList = list;
        queue->_rearTop = top = 0;
    }
    ++queue->count;

    current = &list->levelRuns[top];
    *current = levelRun;

    /* Complete the latest isolating run with this terminating run */
    if (queue->_partialTop != SBInvalidIndex && SBRunKindIsTerminating(current->kind)) {
        SBLevelRunRef incompleteRun = &queue->_partialList->levelRuns[queue->_partialTop];
        SBLevelRunAttach(incompleteRun, current);
        _SBRunQueueFindPreviousPartialRun(queue);
    }

    /* Save the location of the isolating run */
    if (SBRunKindIsIsolate(current->kind)) {
        queue->_partialList = list;
        queue->_partialTop = top;
    }
}

SB_INTERNAL void SBRunQueueDequeue(SBRunQueueRef queue)
{
    /* The queue should not be empty. */
    SBAssert(queue->count != 0);

    if (queue->_frontTop != _SBRunQueueList_MaxIndex) {
        ++queue->_frontTop;
    } else {
        _SBRunQueueListRef frontList = queue->_frontList;
        if (frontList == queue->_rearList) {
            queue->_rearTop = SBInvalidIndex;
        } else {
            queue->_frontList = frontList->next;
        }

        queue->_frontTop = 0;
    }

    --queue->count;
    queue->peek = &queue->_frontList->levelRuns[queue->_frontTop];
}

SB_INTERNAL void SBRunQueueFinalize(SBRunQueueRef queue)
{
    _SBRunQueueListRef list = queue->_firstList.next;

    while (list) {
        _SBRunQueueListRef next = list->next;
        free(list);
        list = next;
    };
}
