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

#ifndef _SB_INTERNAL_LEVEL_RUN_H
#define _SB_INTERNAL_LEVEL_RUN_H

#include <SBConfig.h>
#include <SBTypes.h>

#include "SBCharType.h"
#include "SBBidiLink.h"
#include "SBRunExtrema.h"
#include "SBRunKind.h"

struct _SBLevelRun;
typedef struct _SBLevelRun SBLevelRun;
typedef SBLevelRun *SBLevelRunRef;

struct _SBLevelRun {
    SBLevelRunRef next;             /**< Reference to the next sequence of run links. */
    SBBidiLinkRef firstLink;        /**< First link of the run. */
    SBBidiLinkRef lastLink;         /**< Last link of the run. */
    SBBidiLinkRef subsequentLink;   /**< Subsequent link of the run. */
    SBRunExtrema extrema;
    SBRunKind kind;
};

SB_INTERNAL void SBLevelRunInitialize(SBLevelRunRef levelRun, SBBidiLinkRef firstLink, SBBidiLinkRef lastLink, SBCharType sor, SBCharType eor);
SB_INTERNAL SBLevel SBLevelRunGetLevel(SBLevelRunRef levelRun);
SB_INTERNAL void SBLevelRunAttach(SBLevelRunRef levelRun, SBLevelRunRef next);

#endif
