/*
 * Copyright (C) 2014-2026 Muhammad Tayyab Akram
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

#ifndef _SHEENBIDI_MIRROR_LOCATOR_H
#define _SHEENBIDI_MIRROR_LOCATOR_H

#include <SheenBidi/SBBase.h>
#include <SheenBidi/SBCodepoint.h>
#include <SheenBidi/SBLine.h>

SB_EXTERN_C_BEGIN

/**
 * A reference to a mutable object that locates mirrored code points within a line.
 */
typedef struct _SBMirrorLocator *SBMirrorLocatorRef;

/**
 * A structure containing the information about a code point having the Bidi_Mirrored property.
 */
typedef struct _SBMirrorAgent {
    SBUInteger index;      /**< The absolute index of the code point. */
    SBCodepoint mirror;    /**< The mirrored code point. */
    SBCodepoint codepoint; /**< The actual code point. */
} SBMirrorAgent;

/**
 * Creates a mirror locator object which can be used to find mirrors in a line.
 *
 * @return
 *      A reference to a mirror locator object if the call was successful, `NULL` otherwise.
 */
SB_PUBLIC SBMirrorLocatorRef SBMirrorLocatorCreate(void);

/**
 * Loads a line in the locator so that its mirrors can be located.
 *
 * @param locator
 *      The locator in which the line will be loaded.
 * @param line
 *      The line which will be loaded in the locator.
 * @param stringBuffer
 *      The string buffer from which the line's algorithm was created.
 */
SB_PUBLIC void SBMirrorLocatorLoadLine(SBMirrorLocatorRef locator, SBLineRef line,
    const void *stringBuffer);

/**
 * Returns the agent containing the information of the currently located mirror.
 *
 * The values of the agent are meaningful only after a call to `SBMirrorLocatorMoveNext` has
 * returned `SBTrue`.
 *
 * @param locator
 *      The locator whose agent is returned.
 * @return
 *      A pointer to the locator's mirror agent.
 */
SB_PUBLIC const SBMirrorAgent *SBMirrorLocatorGetAgent(SBMirrorLocatorRef locator);

/**
 * Instructs the locator to find the next mirror in the loaded line.
 *
 * @param locator
 *      The locator to instruct.
 * @return
 *      `SBTrue` if another mirror is available, `SBFalse` otherwise.
 * @note
 *      The locator is reset after locating the last mirror.
 */
SB_PUBLIC SBBoolean SBMirrorLocatorMoveNext(SBMirrorLocatorRef locator);

/**
 * Instructs the locator to reset itself so that mirrors of the loaded line can be obtained from
 * the beginning.
 *
 * @param locator
 *      The locator to reset.
 */
SB_PUBLIC void SBMirrorLocatorReset(SBMirrorLocatorRef locator);

/**
 * Increments the reference count of a mirror locator object.
 *
 * @param locator
 *      The mirror locator object whose reference count will be incremented.
 * @return
 *      The same mirror locator object passed in as the parameter.
 */
SB_PUBLIC SBMirrorLocatorRef SBMirrorLocatorRetain(SBMirrorLocatorRef locator);

/**
 * Decrements the reference count of a mirror locator object. The object will be deallocated when
 * its reference count reaches zero.
 *
 * @param locator
 *      The mirror locator object whose reference count will be decremented.
 */
SB_PUBLIC void SBMirrorLocatorRelease(SBMirrorLocatorRef locator);

SB_EXTERN_C_END

#endif
