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

#ifndef _SB_INTERNAL_ISOLATING_RUN_H
#define _SB_INTERNAL_ISOLATING_RUN_H

#include <SBConfig.h>
#include <SBTypes.h>

#include "SBCharType.h"
#include "SBBidiLink.h"
#include "SBLevelRun.h"
#include "SBBracketQueue.h"

struct _SBIsolatingRun;
typedef struct _SBIsolatingRun SBIsolatingRun;
typedef SBIsolatingRun *SBIsolatingRunRef;

struct _SBIsolatingRun {
    SBUnichar *characters;
    SBLevelRunRef baseLevelRun;
    SBLevelRunRef _lastLevelRun;
    SBBracketQueue _bracketQueue;
    SBBidiLink _dummyLink;
    SBCharType _sos;
    SBCharType _eos;
    SBLevel paragraphLevel;
};

SB_INTERNAL void SBIsolatingRunInitialize(SBIsolatingRunRef isolatingRun);
SB_INTERNAL void SBIsolatingRunResolve(SBIsolatingRunRef isolatingRun);

SB_INTERNAL void SBIsolatingRunFinalize(SBIsolatingRunRef isolatingRun);

#endif
