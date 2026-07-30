/*
 * Copyright (C) 2018-2026 Muhammad Tayyab Akram
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

#include <SheenBidi/SBBase.h>
#include <SheenBidi/SBCodepointSequence.h>
#include <SheenBidi/SBScript.h>

SB_EXTERN_C_BEGIN

/**
 * A reference to a mutable object that locates script runs within a code point sequence.
 */
typedef struct _SBScriptLocator *SBScriptLocatorRef;

/**
 * A structure containing the information about a run of code points having the same script.
 */
typedef struct _SBScriptAgent {
    SBUInteger offset; /**< The index to the first code unit of the run in the source string. */
    SBUInteger length; /**< The number of code units covering the length of the run. */
    SBScript script;   /**< The script of the run. */
} SBScriptAgent;

/**
 * Creates a script locator object which can be used to find script runs in a string.
 *
 * @return
 *      A reference to a script locator object if the call was successful, `NULL` otherwise.
 */
SB_PUBLIC SBScriptLocatorRef SBScriptLocatorCreate(void);

/**
 * Loads a code point sequence in the locator so that its script runs can be located.
 *
 * @param locator
 *      The locator in which the code point sequence will be loaded.
 * @param codepointSequence
 *      The code point sequence which will be loaded in the locator.
 */
SB_PUBLIC void SBScriptLocatorLoadCodepoints(SBScriptLocatorRef locator,
    const SBCodepointSequence *codepointSequence);

/**
 * Returns the agent containing the information of the currently located script run.
 *
 * The values of the agent are meaningful only after a call to `SBScriptLocatorMoveNext` has
 * returned `SBTrue`.
 *
 * @param locator
 *      The locator whose agent is returned.
 * @return
 *      A pointer to the locator's script agent.
 */
SB_PUBLIC const SBScriptAgent *SBScriptLocatorGetAgent(SBScriptLocatorRef locator);

/**
 * Instructs the locator to find the next script run in the loaded code point sequence.
 *
 * @param locator
 *      The locator to instruct.
 * @return
 *      `SBTrue` if another script run is available, `SBFalse` otherwise.
 * @note
 *      The locator is reset after locating the last script run.
 */
SB_PUBLIC SBBoolean SBScriptLocatorMoveNext(SBScriptLocatorRef locator);

/**
 * Instructs the locator to reset itself so that script runs of the loaded code point sequence
 * can be obtained from the beginning.
 *
 * @param locator
 *      The locator to reset.
 */
SB_PUBLIC void SBScriptLocatorReset(SBScriptLocatorRef locator);

/**
 * Increments the reference count of a script locator object.
 *
 * @param locator
 *      The script locator object whose reference count will be incremented.
 * @return
 *      The same script locator object passed in as the parameter.
 */
SB_PUBLIC SBScriptLocatorRef SBScriptLocatorRetain(SBScriptLocatorRef locator);

/**
 * Decrements the reference count of a script locator object. The object will be deallocated when
 * its reference count reaches zero.
 *
 * @param locator
 *      The script locator object whose reference count will be decremented.
 */
SB_PUBLIC void SBScriptLocatorRelease(SBScriptLocatorRef locator);

SB_EXTERN_C_END

#endif
