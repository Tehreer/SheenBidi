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
#include "SBLevelRun.h"
#include "SBRunChain.h"
#include "SBRunLink.h"
#include "SBIsolatingRun.h"

SB_INTERNAL void _SBIsolatingRunInitialize(_SBIsolatingRunRef isolatingRun, _SBRunChainRef chain) {
    isolatingRun->_chain = chain;
    isolatingRun->firstRun = NULL;
    isolatingRun->lastRun = NULL;
    isolatingRun->sos = _SB_CHAR_TYPE__NIL;
    isolatingRun->eos = _SB_CHAR_TYPE__NIL;
}

SB_INTERNAL SBLevel _SBIsolatingRunGetLevel(_SBIsolatingRunRef isolatingRun) {
    return _SBLevelRunGetLevel(isolatingRun->firstRun);
}

SB_INTERNAL void _SBIsolatingRunBuild(_SBIsolatingRunRef isolatingRun, _SBLevelRunRef levelRun, SBLevel baseLevel) {
    _SBRunChainRef chain;
    _SBLevelRunRef current;
    _SBLevelRunRef next;

    chain = isolatingRun->_chain;

    /*
     * The last link of the chain must be empty for isolating run to be built.
     */
    SBAssert(chain->lastLink->type == _SB_CHAR_TYPE__NIL);

    /*
     * Iterate over the level runs and attach their links.
     */
    for (current = levelRun; (next = current->next); current = next) {
        _SBRunLinkReplaceNext(current->lastLink, next->firstLink);
    }
    _SBRunLinkReplaceNext(current->lastLink, chain->lastLink);

    isolatingRun->firstRun = levelRun;
    isolatingRun->lastRun = current;
    isolatingRun->sos = _SB_RUN_EXTREMA__GET_SOR(levelRun->extrema);

    if (!_SB_RUN_KIND__IS_PARTIAL_ISOLATE(levelRun->kind)) {
        isolatingRun->eos = _SB_RUN_EXTREMA__GET_EOR(current->extrema);
    } else {
        SBLevel runLevel = _SBIsolatingRunGetLevel(isolatingRun);
        SBLevel eosLevel = (runLevel > baseLevel ? runLevel : baseLevel);
        isolatingRun->eos = ((eosLevel & 1) ? _SB_CHAR_TYPE__R : _SB_CHAR_TYPE__L);
    }
}

SB_INTERNAL void _SBIsolatingRunInvalidate(_SBIsolatingRunRef isolatingRun) {
    _SBLevelRunRef current;

    /*
     * Iterate over the level runs and attach original subsequent links with
     * last links.
     */
    for (current = isolatingRun->firstRun; current; current = current->next) {
        _SBRunLinkReplaceNext(current->lastLink, current->subsequentLink);
    }

    isolatingRun->firstRun = NULL;
    isolatingRun->lastRun = NULL;
    isolatingRun->sos = _SB_CHAR_TYPE__NIL;
    isolatingRun->eos = _SB_CHAR_TYPE__NIL;
}

SB_INTERNAL _SBRunLinkIter _SBIsolatingRunGetIter(_SBIsolatingRunRef isolatingRun) {
    return _SBRunLinkIterMake(isolatingRun->firstRun->firstLink, isolatingRun->lastRun->lastLink);
}
