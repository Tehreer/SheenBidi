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

#ifndef _SB_LEVEL_RUN_H
#define _SB_LEVEL_RUN_H

#include <SBConfig.h>
#include <SBTypes.h>

#include "SBCharType.h"
#include "SBRunLink.h"
#include "SBRunExtrema.h"
#include "SBRunKind.h"

struct __SBLevelRun;
typedef struct __SBLevelRun _SBLevelRun;
typedef _SBLevelRun *_SBLevelRunRef;

struct __SBLevelRun {
    _SBLevelRunRef next;            /**< Reference to the next sequence of run links */
    _SBRunLinkRef firstLink;        /**< First link of the run. */
    _SBRunLinkRef lastLink;         /**< Last link of the run. */
    _SBRunLinkRef subsequentLink;   /**< Subsequent link of the run. */
    _SBRunExtrema extrema;
    _SBRunKind kind;
};

SB_INTERNAL void _SBLevelRunInitialize(_SBLevelRunRef levelRun, _SBRunLinkRef firstLink, _SBRunLinkRef lastLink, _SBCharType sor, _SBCharType eor);
SB_INTERNAL SBLevel _SBLevelRunGetLevel(_SBLevelRunRef levelRun);
SB_INTERNAL void _SBLevelRunAttach(_SBLevelRunRef levelRun, _SBLevelRunRef next);

#endif
