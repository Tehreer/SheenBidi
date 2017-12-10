/*
 * Copyright (C) 2017 Muhammad Tayyab Akram
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

#ifndef _SB_INTERNAL_BIDI_CHAIN_H
#define _SB_INTERNAL_BIDI_CHAIN_H

#include <SBConfig.h>
#include "SBBase.h"

typedef SBUInt32 SBBidiLink;

#define SBBidiLinkNone (SBUInt32)(-1)

typedef struct _SBBidiChain {
    SBBidiType *types;
    SBLevel *levels;
    SBBidiLink *links;
    SBBidiLink roller;
    SBBidiLink last;
} SBBidiChain, *SBBidiChainRef;

SB_INTERNAL void SBBidiChainInitialize(SBBidiChainRef chain,
    SBBidiType *types, SBLevel *levels, SBBidiLink *links);
SB_INTERNAL void SBBidiChainAdd(SBBidiChainRef chain, SBBidiType type, SBUInteger length);

SB_INTERNAL SBUInteger SBBidiChainGetOffset(SBBidiChainRef chain, SBBidiLink link);
SB_INTERNAL SBBoolean SBBidiChainIsSingle(SBBidiChainRef chain, SBBidiLink link);

SB_INTERNAL SBBidiType SBBidiChainGetType(SBBidiChainRef chain, SBBidiLink link);
SB_INTERNAL void SBBidiChainSetType(SBBidiChainRef chain, SBBidiLink link, SBBidiType type);

SB_INTERNAL SBLevel SBBidiChainGetLevel(SBBidiChainRef chain, SBBidiLink link);
SB_INTERNAL void SBBidiChainSetLevel(SBBidiChainRef chain, SBBidiLink link, SBLevel level);

SB_INTERNAL SBBidiLink SBBidiChainGetNext(SBBidiChainRef chain, SBBidiLink link);
SB_INTERNAL void SBBidiChainSetNext(SBBidiChainRef chain, SBBidiLink link, SBBidiLink next);
SB_INTERNAL void SBBidiChainAbandonNext(SBBidiChainRef chain, SBBidiLink link);
SB_INTERNAL SBBoolean SBBidiChainMergeIfEqual(SBBidiChainRef chain, SBBidiLink first, SBBidiLink second);

#define SBBidiChainForEach(chain, roller, link) \
    for (link = chain->links[roller]; link != roller; link = chain->links[link])

#endif
