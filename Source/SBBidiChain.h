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

#ifndef _SB_INTERNAL_BIDI_CHAIN_H
#define _SB_INTERNAL_BIDI_CHAIN_H

#include <SBConfig.h>
#include "SBBidiLink.h"

typedef struct _SBBidiChain {
    SBBidiLink _dummy;
    SBBidiLinkRef rollerLink;
    SBBidiLinkRef lastLink;
} SBBidiChain, *SBBidiChainRef;

SB_INTERNAL void SBBidiChainInitialize(SBBidiChainRef chain);
SB_INTERNAL void SBBidiChainAddLink(SBBidiChainRef chain, SBBidiLinkRef link);

#endif
