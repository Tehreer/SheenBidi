/*
 * Copyright (C) 2017 Muhammad Tayyab Akram
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
#include <stddef.h>

#include "SBAssert.h"
#include "SBBidiChain.h"
#include "SBBidiType.h"
#include "SBRunExtrema.h"
#include "SBRunKind.h"
#include "SBLevelRun.h"

SB_INTERNAL void SBLevelRunInitialize(SBLevelRunRef levelRun,
    SBBidiChainRef bidiChain, SBBidiLink firstLink, SBBidiLink lastLink,
    SBBidiType sor, SBBidiType eor)
{
    SBBidiType firstType = SBBidiChainGetType(bidiChain, firstLink);
    SBBidiType lastType = SBBidiChainGetType(bidiChain, lastLink);

    levelRun->next = NULL;
    levelRun->firstLink = firstLink;
    levelRun->lastLink = lastLink;
    levelRun->subsequentLink = SBBidiChainGetNext(bidiChain, lastLink);
    levelRun->extrema = SBRunExtremaMake(sor, eor);
    levelRun->kind = SBRunKindMake
                     (
                        SBBidiTypeIsIsolateInitiator(lastType),
                        SBBidiTypeIsIsolateTerminator(firstType)
                     );
    levelRun->level = SBBidiChainGetLevel(bidiChain, firstLink);
}

SB_INTERNAL void SBLevelRunAttach(SBLevelRunRef levelRun, SBLevelRunRef next)
{
    /* Only the runs of same level can be attached. */
    SBAssert(levelRun->level == next->level);
    /* No other run can be attached with a simple run. */
    SBAssert(!SBRunKindIsSimple(levelRun->kind));
    /* No other run can be attached with a complete isolating run. */
    SBAssert(!SBRunKindIsCompleteIsolate(levelRun->kind));
    /* Only a terminating run can be attached with an isolating run. */
    SBAssert(SBRunKindIsIsolate(levelRun->kind) && SBRunKindIsTerminating(next->kind));
    /* The next run must be unattached. */
    SBAssert(!SBRunKindIsAttachedTerminating(next->kind));
    
    if (SBRunKindIsTerminating(next->kind)) {
        SBRunKindMakeAttached(next->kind);
    }
    
    if (SBRunKindIsIsolate(levelRun->kind)) {
        SBRunKindMakeComplete(levelRun->kind);
    }
    
    levelRun->next = next;
}
