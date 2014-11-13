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
#include "SBCharType.h"
#include "SBStatusStack.h"

SB_INTERNAL void _SBStatusStackInitialize(_SBStatusStackRef stack) {
    stack->_firstList.previous = NULL;
    stack->_firstList.next = NULL;
    
    _SBStatusStackSetEmpty(stack);
}

SB_INTERNAL void _SBStatusStackPush(_SBStatusStackRef stack, SBLevel embeddingLevel, _SBCharType overrideStatus, SBBoolean isolateStatus) {
    __SBStatusStackListRef list;
    SBUInteger top;

    /*
     * The stack can hold upto 127 elements.
     */
    SBAssert(stack->count <= 127);

    if (stack->_peekTop != __SB_STATUS_STACK_LIST__MAX_INDEX) {
        list = stack->_peekList;
        top = ++stack->_peekTop;
    } else {
        __SBStatusStackListRef peekList;

        peekList = stack->_peekList;
        list = peekList->next;

        if (!list) {
            list = malloc(sizeof(__SBStatusStackList));
            list->previous = peekList;
            list->next = NULL;

            peekList->next = list;
        }

        stack->_peekList = list;
        stack->_peekTop = top = 0;
    }
    ++stack->count;

    list->embeddingLevel[top]= embeddingLevel;
    list->overrideStatus[top]= overrideStatus;
    list->isolateStatus[top]= isolateStatus;
}

SB_INTERNAL void _SBStatusStackPop(_SBStatusStackRef stack) {
    /*
     * The stack should not be empty.
     */
    SBAssert(stack->count != 0);

    if (stack->_peekTop != 0) {
        --stack->_peekTop;
    } else {
        stack->_peekList = stack->_peekList->previous;
        stack->_peekTop = __SB_STATUS_STACK_LIST__MAX_INDEX;
    }
    --stack->count;
}

SB_INTERNAL void _SBStatusStackSetEmpty(_SBStatusStackRef stack) {
    stack->_peekList = &stack->_firstList;
    stack->_peekTop = 0;
    stack->count = 0;
}

SB_INTERNAL SBLevel _SBStatusStackGetEmbeddingLevel(_SBStatusStackRef stack) {
    return stack->_peekList->embeddingLevel[stack->_peekTop];
}

SB_INTERNAL _SBCharType _SBStatusStackGetOverrideStatus(_SBStatusStackRef stack) {
    return stack->_peekList->overrideStatus[stack->_peekTop];
}

SB_INTERNAL SBBoolean _SBStatusStackGetIsolateStatus(_SBStatusStackRef stack) {
    return stack->_peekList->isolateStatus[stack->_peekTop];
}

SB_INTERNAL void _SBStatusStackInvalidate(_SBStatusStackRef stack) {
    __SBStatusStackListRef list = stack->_firstList.next;

    while (list) {
        __SBStatusStackListRef next = list->next;
        free(list);
        list = next;
    };
}
