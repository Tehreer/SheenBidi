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

#ifndef _SB_RUN_CHAIN_H
#define _SB_RUN_CHAIN_H

#include <SBConfig.h>

#include "SBRunLink.h"

struct __SBRunChain;
typedef struct __SBRunChain _SBRunChain;
typedef _SBRunChain *_SBRunChainRef;

struct __SBRunChain {
    _SBRunLink _dummy;
    _SBRunLinkRef rollerLink;
    _SBRunLinkRef lastLink;
};

SB_INTERNAL void _SBRunChainInitialize(_SBRunChainRef chain);
SB_INTERNAL void _SBRunChainAddLink(_SBRunChainRef chain, _SBRunLinkRef link);
SB_INTERNAL _SBRunLinkIter _SBRunChainGetIter(_SBRunChainRef chain);

#endif
