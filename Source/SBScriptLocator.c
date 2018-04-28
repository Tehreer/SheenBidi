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

#include <stddef.h>
#include <stdlib.h>

#include "SBBase.h"
#include "SBCodepointSequence.h"
#include "SBGeneralCategoryLookup.h"
#include "SBPairingLookup.h"
#include "SBScriptLookup.h"
#include "SBScriptStack.h"
#include "SBScriptLocator.h"

static SBBoolean _SBIsSimilarScript(SBScript lhs, SBScript rhs)
{
    return SBScriptIsCommonOrInherited(lhs)
        || SBScriptIsCommonOrInherited(rhs)
        || lhs == rhs;
}

SBScriptLocatorRef SBScriptLocatorCreate(void)
{
    SBScriptLocatorRef locator;

    locator = malloc(sizeof(SBScriptLocator));
    locator->_codepointSequence.stringEncoding = SBStringEncodingUTF8;
    locator->_codepointSequence.stringBuffer = NULL;
    locator->_codepointSequence.stringLength = 0;
    locator->_retainCount = 1;
    SBScriptLocatorReset(locator);

    return locator;
}

void SBScriptLocatorLoadCodepoints(SBScriptLocatorRef locator, const SBCodepointSequence *codepointSequence)
{
    locator->_codepointSequence = *codepointSequence;
    SBScriptLocatorReset(locator);
}

SBScriptAgentRef SBScriptLocatorGetAgent(SBScriptLocatorRef locator)
{
    return &locator->agent;
}

static void _SBResolveScriptRun(SBScriptLocatorRef locator, SBUInteger offset)
{
    const SBCodepointSequence *sequence = &locator->_codepointSequence;
    SBScriptStackRef stack = &locator->_scriptStack;
    SBScript result = SBScriptZYYY;
    SBUInteger current = offset;
    SBUInteger next = offset;
    SBCodepoint codepoint;

    /* Iterate over the code points of specified string buffer. */
    while ((codepoint = SBCodepointSequenceGetCodepointAt(sequence, &next)) != SBCodepointInvalid) {
        SBBoolean isStacked = SBFalse;
        SBScript script;

        script = SBScriptDetermine(codepoint);

        /* Handle paired punctuations in case of a common script. */
        if (script == SBScriptZYYY) {
            SBGeneralCategory generalCategory = SBGeneralCategoryDetermine(codepoint);

            /* Check if current code point is an open punctuation. */
            if (generalCategory == SBGeneralCategoryPS) {
                SBCodepoint mirror = SBPairingDetermineMirror(codepoint);
                if (mirror) {
                    /* A closing pair exists for this punctuation, so push it onto the stack. */
                    SBScriptStackPush(stack, result, mirror);
                }
            }
            /* Check if current code point is a close punctuation. */
            else if (generalCategory == SBGeneralCategoryPE) {
                SBBoolean isMirrored = (SBPairingDetermineMirror(codepoint) != 0);
                if (isMirrored) {
                    /* Find the matching entry in the stack, while popping the unmatched ones. */
                    while (!SBScriptStackIsEmpty(stack)) {
                        SBCodepoint mirror = SBScriptStackGetMirror(stack);
                        if (mirror != codepoint) {
                            SBScriptStackPop(stack);
                        } else {
                            break;
                        }
                    }

                    if (!SBScriptStackIsEmpty(stack)) {
                        isStacked = SBTrue;
                        /* Paired punctuation match the script of enclosing text. */
                        script = SBScriptStackGetScript(stack);
                    }
                }
            }
        }

        if (_SBIsSimilarScript(result, script)) {
            if (SBScriptIsCommonOrInherited(result) && !SBScriptIsCommonOrInherited(script)) {
                /* Set the concrete script of this code point as the result. */
                result = script;
                /* Seal the pending punctuations with the result. */
                SBScriptStackSealPairs(stack, result);
            }

            if (isStacked) {
                /* Pop the paired punctuation from the stack. */
                SBScriptStackPop(stack);
            }
        } else {
            /* The current code point has a different script, so finish the run. */
            break;
        }

        current = next;
    }

    SBScriptStackLeavePairs(stack);

    /* Set the run info in agent. */
    locator->agent.offset = offset;
    locator->agent.length = current - offset;
    locator->agent.script = result;
}

SBBoolean SBScriptLocatorMoveNext(SBScriptLocatorRef locator)
{
    SBUInteger offset = locator->agent.offset + locator->agent.length;

    if (offset < locator->_codepointSequence.stringLength) {
        _SBResolveScriptRun(locator, offset);
        return SBTrue;
    }

    SBScriptLocatorReset(locator);
    return SBFalse;
}

void SBScriptLocatorReset(SBScriptLocatorRef locator)
{
    SBScriptStackReset(&locator->_scriptStack);
    locator->agent.offset = 0;
    locator->agent.length = 0;
    locator->agent.script = SBScriptNil;
}

SBScriptLocatorRef SBScriptLocatorRetain(SBScriptLocatorRef locator)
{
    if (locator) {
        ++locator->_retainCount;
    }

    return locator;
}

void SBScriptLocatorRelease(SBScriptLocatorRef locator)
{
    if (locator && --locator->_retainCount == 0) {
        free(locator);
    }
}
