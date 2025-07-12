/*
 * Copyright (C) 2014-2025 Muhammad Tayyab Akram
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

#include <SheenBidi/SBConfig.h>

#include "BidiChain.h"
#include "Memory.h"
#include "SBAssert.h"
#include "SBBase.h"
#include "SBCodepoint.h"
#include "BracketQueue.h"

#define MakeBracketQueueIndex()                     \
    { NULL, SBInvalidIndex }
#define SetBracketQueueIndex(bqi, _list, _index)    \
    ((bqi).list = (_list), (bqi).index = (_index))
#define IsInvalidBracketQueueIndex(bqi)             \
    ((bqi).list == NULL)

static const BracketQueueIndex NullBracketQueueIndex = MakeBracketQueueIndex();

#define LIST            0
#define ELEMENTS        1
#define COUNT           2

static SBBoolean BracketQueueInsertElement(BracketQueueRef queue)
{
    SBBoolean isInserted = SBFalse;

    if ((queue->_rearTop + 1) < queue->_rearList->capacity) {
        queue->_rearTop += 1;
        queue->_actualCount += 1;

        isInserted = SBTrue;
    } else {
        BracketQueueListRef previousList = queue->_rearList;
        BracketQueueListRef nextList = previousList->next;

        if (!nextList) {
            const SBUInteger capacity = queue->_actualCount;
            void *pointers[COUNT] = { NULL };
            SBUInteger sizes[COUNT];

            sizes[LIST]     = sizeof(BracketQueueList);
            sizes[ELEMENTS] = sizeof(BracketQueueElement) * capacity;

            if (MemoryAllocateChunks(queue->_memory, MemoryTypeScratch, sizes, COUNT, pointers)) {
                nextList = pointers[LIST];
                nextList->elements = pointers[ELEMENTS];
                nextList->capacity = capacity;
                nextList->previous = previousList;
                nextList->next = NULL;

                previousList->next = nextList;
            }
        }

        if (nextList) {
            queue->_rearList = nextList;
            queue->_rearTop = 0;
            queue->_actualCount += 1;

            isInserted = SBTrue;
        }
    }

    return isInserted;
}

static void MarkOpenPairsAsInvalid(BracketQueueRef queue, BracketQueueIndex from)
{
    BracketQueueListRef rearList = queue->_rearList;
    BracketQueueListRef processedList = NULL;
    SBUInteger invalidCount = 0;
    BracketQueueIndex current;

    current = from;

    do {
        SBUInteger end = (current.list == rearList ? queue->_rearTop + 1 : current.list->capacity);

        while (current.index < end) {
            BracketQueueElement *element = &current.list->elements[current.index];

            if (element->openingLink != BidiLinkNone && element->closingLink == BidiLinkNone) {
                element->openingLink = BidiLinkNone;
                invalidCount += 1;
            }

            current.index += 1;
        }

        processedList = current.list;
        current.list = processedList->next;
        current.index = 0;
    } while (processedList != rearList);

    queue->_actualCount -= invalidCount;
}

static void SkipToNextBracketPair(BracketQueueRef queue)
{
    BracketQueueListRef rearList = queue->_rearList;
    BracketQueueListRef processedList = NULL;
    SBBoolean hasProcessedFirst = SBFalse;
    BracketQueueIndex current;

    current = queue->_front;

    do {
        SBUInteger end = (current.list == rearList ? queue->_rearTop + 1 : current.list->capacity);

        while (current.index < end) {
            BracketQueueElement *element = &current.list->elements[current.index];

            if (element->openingLink != BidiLinkNone && element->closingLink != BidiLinkNone) {
                if (hasProcessedFirst) {
                    queue->_front = current;
                    return;
                }

                queue->pairCount -= 1;
            }

            hasProcessedFirst = SBTrue;
            current.index += 1;
        }

        processedList = current.list;
        current.list = processedList->next;
        current.index = 0;
    } while (processedList != rearList);

    /* All pairs have been skipped at this point. */
    queue->_front = NullBracketQueueIndex;
}

#undef LIST
#undef ELEMENTS
#undef COUNT

SB_INTERNAL void BracketQueueInitialize(BracketQueueRef queue, MemoryRef memory)
{
    queue->_memory = memory;

    queue->_firstList.elements = queue->_elements;
    queue->_firstList.capacity = BracketQueueEmbeddedElementCount;
    queue->_firstList.previous = NULL;
    queue->_firstList.next = NULL;

    BracketQueueReset(queue, SBBidiTypeNil);
}

SB_INTERNAL void BracketQueueReset(BracketQueueRef queue, SBBidiType direction)
{
    queue->_rearList = &queue->_firstList;
    queue->_rearTop = SBInvalidIndex;
    queue->_firstOpenPair = NullBracketQueueIndex;
    queue->_front = NullBracketQueueIndex;
    queue->_actualCount = 0;
    queue->pairCount = 0;
    queue->_direction = direction;
    queue->_isPopulated = SBFalse;
}

SB_INTERNAL void BracketQueueMarkPopulated(BracketQueueRef queue)
{
    SBAssert(!queue->_isPopulated);

    if (queue->pairCount > 0) {
        BracketQueueElement *first = &queue->_firstList.elements[0];

        SetBracketQueueIndex(queue->_front, &queue->_firstList, 0);

        if (first->openingLink == BidiLinkNone
                || first->closingLink == BidiLinkNone) {
            SkipToNextBracketPair(queue);
        }
    }

    queue->_actualCount = 0;
    queue->_isPopulated = SBTrue;
}

SB_INTERNAL SBBoolean BracketQueueEnqueue(BracketQueueRef queue,
    BidiLink priorStrongLink, BidiLink openingLink, SBCodepoint bracket)
{
    SBBoolean isEnqueued = SBFalse;

    /* The queue must NOT be populated yet. */
    SBAssert(!queue->_isPopulated);
    /* The queue can ONLY have a maximum of 63 open pairs. */
    SBAssert(BracketQueueGetOpenPairCount(queue) < BracketQueueMaxOpenPairs);

    if (BracketQueueInsertElement(queue)) {
        BracketQueueElement *element = &queue->_rearList->elements[queue->_rearTop];
        element->priorStrongLink = priorStrongLink;
        element->openingLink = openingLink;
        element->closingLink = BidiLinkNone;
        element->bracket = bracket;
        element->innerStrongType = SBBidiTypeNil;

        if (IsInvalidBracketQueueIndex(queue->_firstOpenPair)) {
            SetBracketQueueIndex(queue->_firstOpenPair, queue->_rearList, queue->_rearTop);
        }

        isEnqueued = SBTrue;
    }

    return isEnqueued;
}

SB_INTERNAL void BracketQueueDequeue(BracketQueueRef queue)
{
    /* The queue MUST be populated and NOT empty. */
    SBAssert(queue->_isPopulated && queue->pairCount > 0);

    SkipToNextBracketPair(queue);
}

SB_INTERNAL SBUInteger BracketQueueGetOpenPairCount(BracketQueueRef queue)
{
    return (queue->_actualCount - queue->pairCount);
}

SB_INTERNAL void BracketQueueAssignInnerStrongType(BracketQueueRef queue, SBBidiType strongType)
{
    BracketQueueIndex first = queue->_firstOpenPair;

    /* The queue must NOT be populated yet. */
    SBAssert(!queue->_isPopulated);
    /* Passed-in bidi type MUST be valid. */
    SBAssert(strongType == SBBidiTypeL || strongType == SBBidiTypeR);

    /* Proceed if at least one open pair exists. */
    if (!IsInvalidBracketQueueIndex(first)) {
        BracketQueueListRef rearList = queue->_rearList;
        BracketQueueListRef processedList = NULL;
        BracketQueueIndex current;

        SetBracketQueueIndex(current, queue->_rearList, queue->_rearTop + 1);

        /* Assign the strong type to all open pairs. */
        do {
            SBUInteger start = (current.list == first.list ? first.index : 0);

            if (current.list != rearList) {
                current.index = current.list->capacity;
            }

            while (current.index-- > start) {
                BracketQueueElement *element = &current.list->elements[current.index];

                if (element->closingLink == BidiLinkNone
                        && element->innerStrongType != queue->_direction) {
                    element->innerStrongType = strongType;
                }
            }

            processedList = current.list;
            current.list = processedList->previous;
        } while (processedList != first.list);
    }
}

SB_INTERNAL void BracketQueueClosePair(BracketQueueRef queue,
    BidiLink closingLink, SBCodepoint bracket)
{
    BracketQueueIndex first = queue->_firstOpenPair;

    /* The queue must NOT be populated yet. */
    SBAssert(!queue->_isPopulated);

    /* Proceed if at least one open pair exists. */
    if (!IsInvalidBracketQueueIndex(first)) {
        BracketQueueListRef rearList = queue->_rearList;
        BracketQueueListRef processedList = NULL;
        BracketQueueIndex current;

        SetBracketQueueIndex(current, rearList, queue->_rearTop + 1);

        /* Find out the matching open pair. */
        do {
            SBUInteger start = (current.list == first.list ? first.index : 0);

            if (current.list != rearList) {
                current.index = current.list->capacity;
            }

            while (current.index-- > start) {
                BracketQueueElement *element = &current.list->elements[current.index];

                if (element->openingLink != BidiLinkNone && element->closingLink == BidiLinkNone
                        && SBCodepointIsCanonicalEquivalentBracket(element->bracket, bracket)) {
                    element->closingLink = closingLink;
                    queue->pairCount += 1;
                    MarkOpenPairsAsInvalid(queue, current);

                    if (queue->_firstOpenPair.list == current.list
                            && queue->_firstOpenPair.index == current.index) {
                        /* No open pair exists at this point. */
                        queue->_firstOpenPair = NullBracketQueueIndex;
                    }
                    return;
                }
            }

            processedList = current.list;
            current.list = processedList->previous;
        } while (processedList != first.list);
    }
}

SB_INTERNAL BidiLink BracketQueueGetPriorStrongLink(BracketQueueRef queue)
{
    return queue->_front.list->elements[queue->_front.index].priorStrongLink;
}

SB_INTERNAL BidiLink BracketQueueGetOpeningLink(BracketQueueRef queue)
{
    return queue->_front.list->elements[queue->_front.index].openingLink;
}

SB_INTERNAL BidiLink BracketQueueGetClosingLink(BracketQueueRef queue)
{
    return queue->_front.list->elements[queue->_front.index].closingLink;
}

SB_INTERNAL SBBidiType BracketQueueGetInnerStrongType(BracketQueueRef queue)
{
    return queue->_front.list->elements[queue->_front.index].innerStrongType;
}
