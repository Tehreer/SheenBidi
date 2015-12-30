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

#include <SBConfig.h>

#include "SBBidiLink.h"
#include "SBBidiChain.h"

SB_INTERNAL void SBBidiChainInitialize(SBBidiChainRef chain) {
    chain->rollerLink = &chain->_dummy;
    chain->rollerLink->next = chain->rollerLink;
    chain->lastLink = chain->rollerLink;

    SBBidiLinkMakeEmpty(chain->rollerLink);
}

SB_INTERNAL void SBBidiChainAddLink(SBBidiChainRef chain, SBBidiLinkRef link) {
    link->next = chain->rollerLink;

    chain->lastLink->next = link;
    chain->lastLink = link;
}
