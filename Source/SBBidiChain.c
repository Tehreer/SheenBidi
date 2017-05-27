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

#include "SBBase.h"
#include "SBCharType.h"
#include "SBBidiChain.h"

SB_INTERNAL void SBBidiChainInitialize(SBBidiChainRef chain,
    SBCharType *types, SBLevel *levels, SBBidiLink *links)
{
    chain->types = types;
    chain->levels = levels;
    chain->links = links;
    chain->roller = 0;
    chain->last = 0;

    /* Make first link empty. */
    chain->types[0] = SBCharTypeNil;
    chain->levels[0] = SBLevelInvalid;
    chain->links[0] = SBBidiLinkNone;
}

SB_INTERNAL void SBBidiChainAdd(SBBidiChainRef chain, SBCharType type, SBUInteger length)
{
    SBBidiLink last = chain->last;
    SBBidiLink current = last + length;

    chain->types[current] = type;
    chain->links[current] = chain->roller;

    chain->links[last] = current;
    chain->last = current;
}

SB_INTERNAL SBUInteger SBBidiChainGetOffset(SBBidiChainRef chain, SBBidiLink link)
{
    return (link - 1);
}

SB_INTERNAL SBBoolean SBBidiChainIsSingle(SBBidiChainRef chain, SBBidiLink link)
{
    SBBidiLink next = chain->links[link];

    /* Check the type of in between code units. */
    while (++link != next) {
        if (chain->types[link] != SBCharTypeBN) {
            return SBFalse;
        }
    }

    return SBTrue;
}

SB_INTERNAL SBCharType SBBidiChainGetType(SBBidiChainRef chain, SBBidiLink link)
{
    return chain->types[link];
}

SB_INTERNAL void SBBidiChainSetType(SBBidiChainRef chain, SBBidiLink link, SBCharType type)
{
    chain->types[link] = type;
}

SB_INTERNAL SBLevel SBBidiChainGetLevel(SBBidiChainRef chain, SBBidiLink link)
{
    return chain->levels[link];
}

SB_INTERNAL void SBBidiChainSetLevel(SBBidiChainRef chain, SBBidiLink link, SBLevel level)
{
    chain->levels[link] = level;
}

SB_INTERNAL SBBidiLink SBBidiChainGetNext(SBBidiChainRef chain, SBBidiLink link)
{
    return chain->links[link];
}

SB_INTERNAL void SBBidiChainSetNext(SBBidiChainRef chain, SBBidiLink link, SBBidiLink next)
{
    chain->links[link] = next;
}

SB_INTERNAL void SBBidiChainAbandonNext(SBBidiChainRef chain, SBBidiLink link)
{
    SBBidiLink next = chain->links[link];
    SBBidiLink limit = chain->links[next];

    chain->links[link] = limit;
}

SB_INTERNAL SBBoolean SBBidiChainMergeIfEqual(SBBidiChainRef chain, SBBidiLink first, SBBidiLink second)
{
    if (chain->types[first] == chain->types[second]
        && chain->levels[first] == chain->levels[second]) {
        chain->links[first] = chain->links[second];
        return SBTrue;
    }

    return SBFalse;
}
