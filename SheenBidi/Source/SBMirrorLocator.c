/*
 * Copyright (C) 2016 Muhammad Tayyab Akram
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

#include <SBTypes.h>

#include <stddef.h>
#include <stdlib.h>

#include "SBLine.h"
#include "SBPairingLookup.h"
#include "SBMirrorLocator.h"

SBMirrorLocatorRef SBMirrorLocatorCreate(void)
{
    SBMirrorLocatorRef locator;

    locator = malloc(sizeof(SBMirrorLocator));
    locator->_retainCount = 1;
    locator->_refSource = NULL;
    locator->_line = NULL;
    SBMirrorLocatorReset(locator);

    return locator;
}

void SBMirrorLocatorLoadLine(SBMirrorLocatorRef locator, SBLineRef line, void *source)
{
    SBLineRelease(locator->_line);

    locator->_refSource = source;
    locator->_line = SBLineRetain(line);
}

const SBMirrorAgentRef SBMirrorLocatorGetAgent(SBMirrorLocatorRef locator)
{
    return &locator->agent;
}

SBBoolean SBMirrorLocatorMoveNext(SBMirrorLocatorRef locator)
{
    SBCodepoint *codepoints = locator->_refSource;
    SBLineRef line = locator->_line;

    do {
        SBRunRef run = &line->fixedRuns[locator->_runIndex];

        if (run->level & 1) {
            SBUInteger index;
            SBUInteger limit;

            index = locator->_charIndex;
            if (index == SBInvalidIndex) {
                index = run->offset;
            }
            limit = run->offset + run->length;

            for (; index < limit; index++) {
                SBCodepoint mirror = SBPairingDetermineMirror(codepoints[index]);

                if (mirror) {
                    locator->_charIndex = index + 1;
                    locator->agent.index = index;
                    locator->agent.mirror = mirror;
                    
                    return SBTrue;
                }
            }
        }

        locator->_charIndex = SBInvalidIndex;
    } while (++locator->_runIndex < line->runCount);

    SBMirrorLocatorReset(locator);
    
    return SBFalse;
}

void SBMirrorLocatorReset(SBMirrorLocatorRef locator)
{
    locator->_runIndex = 0;
    locator->_charIndex = SBInvalidIndex;
    locator->agent.index = SBInvalidIndex;
    locator->agent.mirror = 0;
}

SBMirrorLocatorRef SBMirrorLocatorRetain(SBMirrorLocatorRef locator)
{
    if (locator) {
        ++locator->_retainCount;
    }

    return locator;
}

void SBMirrorLocatorRelease(SBMirrorLocatorRef locator)
{
    if (locator && --locator->_retainCount == 0) {
        SBLineRelease(locator->_line);
        free(locator);
    }
}
