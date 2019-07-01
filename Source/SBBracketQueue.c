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

#include <SBConfig.h>
#include <stddef.h>
#include <stdlib.h>

#include "SBAssert.h"
#include "SBBase.h"
#include "SBBidiChain.h"
#include "SBBracketQueue.h"

static void BracketQueueFinalizePairs(SBBracketQueueRef queue, BracketQueueListRef list, SBInteger top)
{
    do {
        SBInteger limit = (list == queue->_rearList ? queue->_rearTop : BracketQueueList_MaxIndex);

        while (++top <= limit) {
            if (list->openingLink[top] != SBBidiLinkNone
                && list->closingLink[top] == SBBidiLinkNone) {
                list->openingLink[top] = SBBidiLinkNone;
            }
        }

        list = list->next;
        top = 0;
    } while (list);
}

SB_INTERNAL void SBBracketQueueInitialize(SBBracketQueueRef queue)
{
    queue->_firstList.previous = NULL;
    queue->_firstList.next = NULL;
    queue->_frontList = NULL;
    queue->_rearList = NULL;
    queue->count = 0;
    queue->shouldDequeue = SBFalse;
}

SB_INTERNAL void SBBracketQueueReset(SBBracketQueueRef queue, SBBidiType direction)
{
    queue->_frontList = &queue->_firstList;
    queue->_rearList = &queue->_firstList;
    queue->_frontTop = 0;
    queue->_rearTop = -1;
    queue->count = 0;
    queue->shouldDequeue = SBFalse;
    queue->_direction = direction;
}

SB_INTERNAL void SBBracketQueueEnqueue(SBBracketQueueRef queue,
   SBBidiLink priorStrongLink, SBBidiLink openingLink, SBCodepoint bracket)
{
    BracketQueueListRef list;
    SBInteger top;

    /* The queue can only take a maximum of 63 elements. */
    SBAssert(queue->count < SBBracketQueueGetMaxCapacity());

    if (queue->_rearTop != BracketQueueList_MaxIndex) {
        list = queue->_rearList;
        top = ++queue->_rearTop;
    } else {
        BracketQueueListRef rearList;

        rearList = queue->_rearList;
        list = rearList->next;

        if (!list) {
            list = malloc(sizeof(BracketQueueList));
            list->previous = rearList;
            list->next = NULL;

            rearList->next = list;
        }

        queue->_rearList = list;
        queue->_rearTop = top = 0;
    }
    queue->count += 1;

    list->priorStrongLink[top] = priorStrongLink;
    list->openingLink[top] = openingLink;
    list->closingLink[top] = SBBidiLinkNone;
    list->bracket[top] = bracket;
    list->strongType[top] = SBBidiTypeNil;
}

SB_INTERNAL void SBBracketQueueDequeue(SBBracketQueueRef queue)
{
    /* The queue must NOT be empty. */
    SBAssert(queue->count != 0);

    if (queue->_frontTop != BracketQueueList_MaxIndex) {
        queue->_frontTop += 1;
    } else {
        BracketQueueListRef frontList = queue->_frontList;

        if (frontList == queue->_rearList) {
            queue->_rearTop = -1;
        } else {
            queue->_frontList = frontList->next;
        }

        queue->_frontTop = 0;
    }

    queue->count -= 1;
}

SB_INTERNAL void SBBracketQueueSetStrongType(SBBracketQueueRef queue, SBBidiType strongType)
{
    BracketQueueListRef list = queue->_rearList;
    SBInteger top = queue->_rearTop;

    while (1) {
        SBInteger limit = (list == queue->_frontList ? queue->_frontTop : 0);

        do {
            if (list->closingLink[top] == SBBidiLinkNone
                && list->strongType[top] != queue->_direction) {
                list->strongType[top] = strongType;
            }
        } while (top-- > limit);

        if (list == queue->_frontList) {
            break;
        }

        list = list->previous;
        top = BracketQueueList_MaxIndex;
    }
}

SB_INTERNAL void SBBracketQueueClosePair(SBBracketQueueRef queue, SBBidiLink closingLink, SBCodepoint bracket)
{
    BracketQueueListRef list = queue->_rearList;
    SBInteger top = queue->_rearTop;
    SBCodepoint canonical;

    switch (bracket) {
    case 0x232A:
        canonical = 0x3009;
        break;

    case 0x3009:
        canonical = 0x232A;
        break;

    default:
        canonical = bracket;
        break;
    }

    while (1) {
        SBBoolean isFrontList = (list == queue->_frontList);
        SBInteger limit = (isFrontList ? queue->_frontTop : 0);

        do {
            if (list->openingLink[top] != SBBidiLinkNone
                && list->closingLink[top] == SBBidiLinkNone
                && (list->bracket[top] == bracket || list->bracket[top] == canonical)) {
                list->closingLink[top] = closingLink;
                BracketQueueFinalizePairs(queue, list, top);

                if (isFrontList && top == queue->_frontTop) {
                    queue->shouldDequeue = SBTrue;
                }

                return;
            }
        } while (top-- > limit);

        if (isFrontList) {
            break;
        }

        list = list->previous;
        top = BracketQueueList_MaxIndex;
    }
}

SB_INTERNAL SBBoolean SBBracketQueueShouldDequeue(SBBracketQueueRef queue)
{
    return queue->shouldDequeue;
}

SB_INTERNAL SBBidiLink SBBracketQueueGetPriorStrongLink(SBBracketQueueRef queue)
{
    return queue->_frontList->priorStrongLink[queue->_frontTop];
}

SB_INTERNAL SBBidiLink SBBracketQueueGetOpeningLink(SBBracketQueueRef queue)
{
    return queue->_frontList->openingLink[queue->_frontTop];
}

SB_INTERNAL SBBidiLink SBBracketQueueGetClosingLink(SBBracketQueueRef queue)
{
    return queue->_frontList->closingLink[queue->_frontTop];
}

SB_INTERNAL SBBidiType SBBracketQueueGetStrongType(SBBracketQueueRef queue)
{
    return queue->_frontList->strongType[queue->_frontTop];
}

SB_INTERNAL void SBBracketQueueFinalize(SBBracketQueueRef queue)
{
    BracketQueueListRef list = queue->_firstList.next;

    while (list) {
        BracketQueueListRef next = list->next;
        free(list);
        list = next;
    }
}
