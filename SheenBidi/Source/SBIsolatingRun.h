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

#ifndef _SB_INTERNAL_ISOLATING_RUN_H
#define _SB_INTERNAL_ISOLATING_RUN_H

#include <SBConfig.h>

#include "SBBase.h"
#include "SBBidiLink.h"
#include "SBBracketQueue.h"
#include "SBCharType.h"
#include "SBCodepointSequence.h"
#include "SBLevelRun.h"

typedef struct _SBIsolatingRun {
    SBCodepointSequenceRef codepointSequence;
    SBLevelRunRef baseLevelRun;
    SBLevelRunRef _lastLevelRun;
    SBBracketQueue _bracketQueue;
    SBBidiLink _dummyLink;
    SBCharType _sos;
    SBCharType _eos;
    SBLevel paragraphLevel;
} SBIsolatingRun, *SBIsolatingRunRef;

SB_INTERNAL void SBIsolatingRunInitialize(SBIsolatingRunRef isolatingRun);
SB_INTERNAL void SBIsolatingRunResolve(SBIsolatingRunRef isolatingRun);

SB_INTERNAL void SBIsolatingRunFinalize(SBIsolatingRunRef isolatingRun);

#endif
