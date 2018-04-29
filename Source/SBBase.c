/*
 * Copyright (C) 2016-2018 Muhammad Tayyab Akram
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

#include "SBBase.h"
#include "SBBidiTypeLookup.h"
#include "SBPairingLookup.h"
#include "SBScriptLookup.h"

SB_INTERNAL void SBUIntegerNormalizeRange(SBUInteger actualLength,
    SBUInteger *rangeOffset, SBUInteger *rangeLength)
{
    /**
     * Assume:
     *      Actual Length = 10
     *
     * Case 1:
     *      Offset = 0, Length = 10
     * Result:
     *      Offset = 0, Length = 10
     *
     * Case 2:
     *      Offset = 0, Length = 11
     * Result:
     *      Offset = 0, Length = 10
     *
     * Case 3:
     *      Offset = 1, Length = -1 (MAX)
     * Result:
     *      Offset = 1, Length = 9
     *
     * Case 4:
     *      Offset = 10, Length = 0
     * Result:
     *      Offset = Invalid, Length = 0
     *
     * Case 5:
     *      Offset = -1 (MAX), Length = 1
     * Result:
     *      Offset = Invalid, Length = 0
     */

    if (*rangeOffset < actualLength) {
        SBUInteger possibleLimit = *rangeOffset + *rangeLength;

        if (*rangeOffset <= possibleLimit && possibleLimit <= actualLength) {
            /* The range is valid. Nothing to do here. */
        } else {
            *rangeLength = actualLength - *rangeOffset;
        }
    } else {
        *rangeOffset = SBInvalidIndex;
        *rangeLength = 0;
    }
}

SB_INTERNAL SBBoolean SBUIntegerVerifyRange(SBUInteger actualLength,
    SBUInteger rangeOffset, SBUInteger rangeLength)
{
    SBUInteger possibleLimit = rangeOffset + rangeLength;

    return rangeOffset < actualLength
        && rangeOffset <= possibleLimit
        && possibleLimit <= actualLength;
}

SBBidiType SBCodepointGetBidiType(SBCodepoint codepoint)
{
    return SBBidiTypeDetermine(codepoint);
}

SBCodepoint SBCodepointGetMirror(SBCodepoint codepoint)
{
    return SBPairingDetermineMirror(codepoint);
}

SBScript SBCodepointGetScript(SBCodepoint codepoint)
{
    return SBScriptDetermine(codepoint);
}
