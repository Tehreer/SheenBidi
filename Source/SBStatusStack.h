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

#ifndef _SB_STATUS_STACK_H
#define _SB_STATUS_STACK_H

#include <SBConfig.h>
#include <SBTypes.h>

#include "SBCharType.h"

#define __SB_STATUS_STACK_LIST__LENGTH      16
#define __SB_STATUS_STACK_LIST__MAX_INDEX   (__SB_STATUS_STACK_LIST__LENGTH - 1)

struct __SBStatusStack;
struct __SBStatusStackList;

typedef struct __SBStatusStack _SBStatusStack;
typedef struct __SBStatusStackList __SBStatusStackList;

typedef _SBStatusStack *_SBStatusStackRef;
typedef __SBStatusStackList *__SBStatusStackListRef;

struct __SBStatusStackList {
    __SBStatusStackListRef previous;
    __SBStatusStackListRef next;

    SBBoolean isolateStatus[__SB_STATUS_STACK_LIST__LENGTH];
    _SBCharType overrideStatus[__SB_STATUS_STACK_LIST__LENGTH];
    SBLevel embeddingLevel[__SB_STATUS_STACK_LIST__LENGTH];
};

struct __SBStatusStack {
    __SBStatusStackList _firstList;
    __SBStatusStackListRef _peekList;
    SBUInteger _peekTop;
    SBUInteger count;
};

SB_INTERNAL void _SBStatusStackInitialize(_SBStatusStackRef stack);
SB_INTERNAL void _SBStatusStackInvalidate(_SBStatusStackRef stack);

SB_INTERNAL void _SBStatusStackPush(_SBStatusStackRef stack, SBLevel embeddingLevel, _SBCharType overrideStatus, SBBoolean isolateStatus);
SB_INTERNAL void _SBStatusStackPop(_SBStatusStackRef stack);
SB_INTERNAL void _SBStatusStackSetEmpty(_SBStatusStackRef stack);

SB_INTERNAL SBLevel _SBStatusStackGetEmbeddingLevel(_SBStatusStackRef stack);
SB_INTERNAL _SBCharType _SBStatusStackGetOverrideStatus(_SBStatusStackRef stack);
SB_INTERNAL SBBoolean _SBStatusStackGetIsolateStatus(_SBStatusStackRef stack);

#endif
