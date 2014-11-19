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

#include <stddef.h>
#include <SBConfig.h>

#include "SBAssert.h"
#include "SBCharType.h"
#include "SBBidiLink.h"
#include "SBRunExtrema.h"
#include "SBRunKind.h"
#include "SBLevelRun.h"

SB_INTERNAL void _SBLevelRunInitialize(_SBLevelRunRef levelRun, _SBBidiLinkRef firstLink, _SBBidiLinkRef lastLink, _SBCharType sor, _SBCharType eor) {
    levelRun->next = NULL;
    levelRun->firstLink = firstLink;
    levelRun->lastLink = lastLink;
    levelRun->subsequentLink = lastLink->next;
    levelRun->extrema = _SB_RUN_EXTREMA__MAKE(sor, eor);
    levelRun->kind = _SB_RUN_KIND__MAKE
                     (
                        _SB_CHAR_TYPE__IS_ISOLATE_INITIATOR(lastLink->type),
                        _SB_CHAR_TYPE__IS_ISOLATE_TERMINATOR(firstLink->type)
                     );
}

SB_INTERNAL SBLevel _SBLevelRunGetLevel(_SBLevelRunRef levelRun) {
    return levelRun->firstLink->level;
}

SB_INTERNAL void _SBLevelRunAttach(_SBLevelRunRef levelRun, _SBLevelRunRef next) {
    /*
     * Only the runs of same level can be attached.
     */
    SBAssert(_SBLevelRunGetLevel(levelRun) == _SBLevelRunGetLevel(next));
    /*
     * No other run can be attached with a simple run.
     */
    SBAssert(!_SB_RUN_KIND__IS_SIMPLE(levelRun->kind));
    /*
     * No other run can be attached with a complete isolating run.
     */
    SBAssert(!_SB_RUN_KIND__IS_COMPLETE_ISOLATE(levelRun->kind));
    /*
     * Only a terminating run can be attached with an isolating run.
     */
    SBAssert(_SB_RUN_KIND__IS_ISOLATE(levelRun->kind) && _SB_RUN_KIND__IS_TERMINATING(next->kind));
    /*
     * The next run must be unattached.
     */
    SBAssert(!_SB_RUN_KIND__IS_ATTACHED_TERMINATING(next->kind));
    
    if (_SB_RUN_KIND__IS_TERMINATING(next->kind)) {
        _SB_RUN_KIND__MAKE_ATTACHED(next->kind);
    }
    
    if (_SB_RUN_KIND__IS_ISOLATE(levelRun->kind)) {
        _SB_RUN_KIND__MAKE_COMPLETE(levelRun->kind);
    }
    
    levelRun->next = next;
}
