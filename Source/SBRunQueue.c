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

#include <stddef.h>
#include <stdlib.h>

#include <SBConfig.h>
#include <SBTypes.h>

#include "SBAssert.h"
#include "SBLevelRun.h"
#include "SBRunQueue.h"

static void __SBRunQueueFindPreviousPartialRun(_SBRunQueueRef queue) {
    __SBRunQueueListRef list = queue->_partialList;
    SBUInteger top = queue->_partialTop;

    do {
        SBUInteger limit = (list == queue->_frontList ? queue->_frontTop : 0);

        do {
            _SBLevelRunRef levelRun = &list->levelRuns[top];
            if (_SB_RUN_KIND__IS_PARTIAL_ISOLATE(levelRun->kind)) {
                queue->_partialList = list;
                queue->_partialTop = top;
                return;
            }
        } while (top-- > limit);

        list = list->previous;
        top = __SB_RUN_QUEUE_LIST__MAX_INDEX;
    } while (list);

    queue->_partialList = NULL;
    queue->_partialTop = SBInvalidIndex;
    queue->shouldDequeue = SBFalse;
}

SB_INTERNAL void _SBRunQueueInitialize(_SBRunQueueRef queue) {
    /* Initialize first list */
    queue->_firstList.previous = NULL;
    queue->_firstList.next = NULL;

    /* Initialize front and rear lists with first list */
    queue->_frontList = &queue->_firstList;
    queue->_rearList = &queue->_firstList;
    queue->_partialList = NULL;

    /* Initialize list indexes */
    queue->_frontTop = 0;
    queue->_rearTop = SBInvalidIndex;
    queue->_partialTop = SBInvalidIndex;

    /* Initialize rest of the elements */
    queue->count = 0;
    queue->peek = &queue->_frontList->levelRuns[queue->_frontTop];
    queue->shouldDequeue = SBFalse;
}

SB_INTERNAL void _SBRunQueueEnqueue(_SBRunQueueRef queue, _SBLevelRun levelRun) {
    _SBLevelRunRef current;
    __SBRunQueueListRef list;
    SBUInteger top;

    if (queue->_rearTop != __SB_RUN_QUEUE_LIST__MAX_INDEX) {
        list = queue->_rearList;
        top = ++queue->_rearTop;
    } else {
        __SBRunQueueListRef rearList;

        rearList = queue->_rearList;
        list = rearList->next;

        if (!list) {
            list = malloc(sizeof(__SBRunQueueList));
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
    if (queue->_partialTop != SBInvalidIndex && _SB_RUN_KIND__IS_TERMINATING(current->kind)) {
        _SBLevelRunRef incompleteRun = &queue->_partialList->levelRuns[queue->_partialTop];
        _SBLevelRunAttach(incompleteRun, current);
        __SBRunQueueFindPreviousPartialRun(queue);
    }

    /* Save the location of the isolating run */
    if (_SB_RUN_KIND__IS_ISOLATE(current->kind)) {
        queue->_partialList = list;
        queue->_partialTop = top;
    }
}

SB_INTERNAL void _SBRunQueueDequeue(_SBRunQueueRef queue) {
    /*
     * The queue should not be empty.
     */
    SBAssert(queue->count != 0);

    if (queue->_frontTop != __SB_RUN_QUEUE_LIST__MAX_INDEX) {
        ++queue->_frontTop;
    } else {
        __SBRunQueueListRef frontList = queue->_frontList;
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

SB_INTERNAL void _SBRunQueueInvalidate(_SBRunQueueRef queue) {
    __SBRunQueueListRef list = queue->_firstList.next;

    while (list) {
        __SBRunQueueListRef next = list->next;
        free(list);
        list = next;
    };
}
