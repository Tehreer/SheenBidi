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

#ifndef _SB_RUN_LINK_H
#define _SB_RUN_LINK_H

#include <SBConfig.h>
#include <SBTypes.h>

#include "SBCharType.h"

struct __SBRunLink;
struct __SBRunLinkIter;

typedef struct __SBRunLink _SBRunLink;
typedef struct __SBRunLinkIter _SBRunLinkIter;

typedef _SBRunLink *_SBRunLinkRef;
typedef _SBRunLinkIter *_SBRunLinkIterRef;

struct __SBRunLink {
    _SBRunLinkRef next;
    SBUInteger offset;
    SBUInteger length;
    _SBCharType type;
    SBLevel level;
};

struct __SBRunLinkIter {
    _SBRunLinkRef _first;
    _SBRunLinkRef _break;
    _SBRunLinkRef current;
};

SB_INTERNAL void _SBRunLinkMakeEmpty(_SBRunLinkRef link);
SB_INTERNAL void _SBRunLinkAbandonNext(_SBRunLinkRef link);
SB_INTERNAL void _SBRunLinkReplaceNext(_SBRunLinkRef link, _SBRunLinkRef next);
SB_INTERNAL void _SBRunLinkMergeNext(_SBRunLinkRef link);

SB_INTERNAL _SBRunLinkIter _SBRunLinkIterMake(_SBRunLinkRef firstLink, _SBRunLinkRef lastLink);
SB_INTERNAL void _SBRunLinkIterReset(_SBRunLinkIterRef iter);
SB_INTERNAL SBBoolean _SBRunLinkIterMoveNext(_SBRunLinkIterRef iter);

#endif
