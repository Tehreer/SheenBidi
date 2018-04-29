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
    SBInteger top = queue->_partialTop;

    do {
        SBInteger limit = (list == queue->_frontList ? queue->_frontTop : 0);

        do {
            SBLevelRunRef levelRun = &list->elements[top];
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
    queue->_partialTop = -1;
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
    queue->_rearTop = -1;
    queue->_partialTop = -1;

    /* Initialize rest of the elements. */
    queue->count = 0;
    queue->peek = &queue->_frontList->elements[queue->_frontTop];
    queue->shouldDequeue = SBFalse;
}

SB_INTERNAL void SBRunQueueEnqueue(SBRunQueueRef queue, const SBLevelRunRef levelRun)
{
    SBLevelRunRef element;

    if (queue->_rearTop != _SBRunQueueList_MaxIndex) {
        element = &queue->_rearList->elements[++queue->_rearTop];
    } else {
        _SBRunQueueListRef previousList = queue->_rearList;
        _SBRunQueueListRef rearList = previousList->next;

        if (!rearList) {
            rearList = malloc(sizeof(_SBRunQueueList));
            rearList->previous = previousList;
            rearList->next = NULL;

            previousList->next = rearList;
        }

        queue->_rearList = rearList;
        queue->_rearTop = 0;

        element = &rearList->elements[0];
    }
    queue->count += 1;

    /* Copy the level run into current element. */
    *element = *levelRun;

    /* Complete the latest isolating run with this terminating run */
    if (queue->_partialTop != -1 && SBRunKindIsTerminating(element->kind)) {
        SBLevelRunRef incompleteRun = &queue->_partialList->elements[queue->_partialTop];
        SBLevelRunAttach(incompleteRun, element);
        _SBRunQueueFindPreviousPartialRun(queue);
    }

    /* Save the location of the isolating run */
    if (SBRunKindIsIsolate(element->kind)) {
        queue->_partialList = queue->_rearList;
        queue->_partialTop = queue->_rearTop;
    }
}

SB_INTERNAL void SBRunQueueDequeue(SBRunQueueRef queue)
{
    /* The queue should not be empty. */
    SBAssert(queue->count != 0);

    if (queue->_frontTop != _SBRunQueueList_MaxIndex) {
        queue->_frontTop += 1;
    } else {
        _SBRunQueueListRef frontList = queue->_frontList;

        if (frontList == queue->_rearList) {
            queue->_rearTop = -1;
        } else {
            queue->_frontList = frontList->next;
        }

        queue->_frontTop = 0;
    }

    queue->count -= 1;
    queue->peek = &queue->_frontList->elements[queue->_frontTop];
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
