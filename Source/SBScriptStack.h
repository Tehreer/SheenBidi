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

#ifndef _SB_INTERNAL_SCRIPT_STACK_H
#define _SB_INTERNAL_SCRIPT_STACK_H

#include <SBBase.h>
#include <SBConfig.h>

#define _SBScriptStackCapacity  63

typedef struct _SBScriptStackElement {
    SBScript script;
    SBCodepoint mirror;
} _SBScriptStackElement;

typedef struct _SBScriptStack {
    _SBScriptStackElement _elements[_SBScriptStackCapacity];
    SBInteger top;
    SBUInteger count;
    SBUInteger open;
} SBScriptStack, *SBScriptStackRef;

SB_INTERNAL void SBScriptStackInitialize(SBScriptStackRef stack);
SB_INTERNAL void SBScriptStackReset(SBScriptStackRef stack);

SB_INTERNAL void SBScriptStackPush(SBScriptStackRef stack, SBScript script, SBCodepoint mirror);
SB_INTERNAL void SBScriptStackPop(SBScriptStackRef stack);

SB_INTERNAL void SBScriptStackLeavePairs(SBScriptStackRef stack);
SB_INTERNAL void SBScriptStackSealPairs(SBScriptStackRef stack, SBScript script);

SB_INTERNAL SBBoolean SBScriptStackIsEmpty(SBScriptStackRef stack);
SB_INTERNAL SBScript SBScriptStackGetScript(SBScriptStackRef stack);
SB_INTERNAL SBCodepoint SBScriptStackGetMirror(SBScriptStackRef stack);

#endif
