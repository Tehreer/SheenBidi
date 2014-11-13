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

#ifndef SB_MIRROR_LOCATOR_H
#define SB_MIRROR_LOCATOR_H

#include "SBTypes.h"
#include "SBLine.h"

struct __SBMirrorLocator;
typedef struct __SBMirrorLocator SBMirrorLocator;
typedef SBMirrorLocator *SBMirrorLocatorRef;

/**
 * A structure containing the information about a character having Bidi_Mirrored
 * property.
 */
typedef struct {
    SBUInteger index;   /**< The absolute index of a character. */
    SBUnichar mirror;   /**< The mirrored character of an actual character. */
} SBMirrorAgent;
typedef SBMirrorAgent *SBMirrorAgentRef;

/**
 * Creates a mirror locator object.
 * @return
 *      A reference to a mirror locator object.
 */
SBMirrorLocatorRef SBMirrorLocatorCreate(void);
/**
 * Loads a line in the locator so that its mirror can be located.
 * @param locator
 *      The locator in which the line will be loaded.
 * @param line
 *      The line which will be loaded in the locator.
 * @param characters
 *      The pointer to the characters from which the line/paragraph was created.
 */
void SBMirrorLocatorLoadLine(SBMirrorLocatorRef locator, SBLineRef line, void *characters);

/**
 * Provides the agent containing the information of current located mirror.
 * @param locator
 *      The locator whose agent you want to obtain.
 */
const SBMirrorAgentRef SBMirrorLocatorGetAgent(SBMirrorLocatorRef locator);
/**
 * Instructs the locator to locate next mirror in the loaded line.
 * @param locator
 *      The locator whom you want to instruct.
 * @return
 *      SBTrue if another mirror is available, SBFalse otherwise.
 * @note
 *      The locator will be reset after locating last mirror.
 */
SBBoolean SBMirrorLocatorMoveNext(SBMirrorLocatorRef locator);
/**
 * Instructs the locator to reset itself so that mirrors of the loaded line can
 * be obatained from the beginning.
 * @param locator
 *      The locator whom you want to reset.
 */
void SBMirrorLocatorReset(SBMirrorLocatorRef locator);

SBMirrorLocatorRef SBMirrorLocatorRetain(SBMirrorLocatorRef locator);
void SBMirrorLocatorRelease(SBMirrorLocatorRef locator);

#endif
