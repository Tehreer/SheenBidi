/*
 * Copyright (C) 2018 Muhammad Tayyab Akram
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

#ifndef _SB_PUBLIC_SCRIPT_LOCATOR_H
#define _SB_PUBLIC_SCRIPT_LOCATOR_H

#include "SBBase.h"
#include "SBCodepointSequence.h"

typedef struct _SBScriptLocator *SBScriptLocatorRef;

typedef struct _SBScriptAgent {
    SBUInteger offset;
    SBUInteger length;
    SBScript script;
} SBScriptAgent, *SBScriptAgentRef;

SBScriptLocatorRef SBScriptLocatorCreate(void);

void SBScriptLocatorLoadCodepoints(SBScriptLocatorRef locator, const SBCodepointSequence *codepointSequence);

SBScriptAgentRef SBScriptLocatorGetAgent(SBScriptLocatorRef locator);

SBBoolean SBScriptLocatorMoveNext(SBScriptLocatorRef locator);

void SBScriptLocatorReset(SBScriptLocatorRef locator);

SBScriptLocatorRef SBScriptLocatorRetain(SBScriptLocatorRef locator);
void SBScriptLocatorRelease(SBScriptLocatorRef locator);

#endif
