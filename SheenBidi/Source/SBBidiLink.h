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

#ifndef _SB_INTERNAL_BIDI_LINK_H
#define _SB_INTERNAL_BIDI_LINK_H

#include <SBConfig.h>
#include <SBTypes.h>

#include "SBCharType.h"

struct _SBBidiLink;
typedef struct _SBBidiLink SBBidiLink;
typedef SBBidiLink *SBBidiLinkRef;

struct _SBBidiLink {
    SBBidiLinkRef next;
    SBUInteger offset;
    SBUInteger length;
    SBCharType type;
    SBLevel level;
};

SB_INTERNAL void SBBidiLinkMakeEmpty(SBBidiLinkRef link);
SB_INTERNAL void SBBidiLinkAbandonNext(SBBidiLinkRef link);
SB_INTERNAL void SBBidiLinkReplaceNext(SBBidiLinkRef link, SBBidiLinkRef next);
SB_INTERNAL void SBBidiLinkMergeNext(SBBidiLinkRef link);

#endif
