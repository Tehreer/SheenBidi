/*
 * Copyright (C) 2016-2025 Muhammad Tayyab Akram
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
#include <stdlib.h>

#include "SBAssert.h"
#include "SBBase.h"
#include "SBCodepointSequence.h"

SB_INTERNAL SBBoolean SBCodepointSequenceIsValid(const SBCodepointSequence *codepointSequence)
{
    if (codepointSequence) {
        SBBoolean encodingValid = SBFalse;

        switch (codepointSequence->stringEncoding) {
        case SBStringEncodingUTF8:
        case SBStringEncodingUTF16:
        case SBStringEncodingUTF32:
            encodingValid = SBTrue;
            break;
        }

        return (encodingValid && codepointSequence->stringBuffer && codepointSequence->stringLength > 0);
    }

    return SBFalse;
}

SBCodepoint SBCodepointSequenceGetCodepointBefore(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex)
{
    SBCodepoint codepoint = SBCodepointInvalid;

    if ((*stringIndex - 1) < codepointSequence->stringLength) {
        switch (codepointSequence->stringEncoding) {
        case SBStringEncodingUTF8:
            codepoint = SBCodepointDecodePreviousFromUTF8(codepointSequence->stringBuffer, codepointSequence->stringLength, stringIndex);
            break;

        case SBStringEncodingUTF16:
            codepoint = SBCodepointDecodePreviousFromUTF16(codepointSequence->stringBuffer, codepointSequence->stringLength, stringIndex);
            break;

        case SBStringEncodingUTF32:
            codepoint = ((const SBUInt32 *) codepointSequence->stringBuffer)[--(*stringIndex)];
            if (!SBCodepointIsValid(codepoint)) codepoint = SBCodepointFaulty;
            break;
        }
    }

    return codepoint;
}

SBCodepoint SBCodepointSequenceGetCodepointAt(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex)
{
    SBCodepoint codepoint = SBCodepointInvalid;

    if (*stringIndex < codepointSequence->stringLength) {
        switch (codepointSequence->stringEncoding) {
        case SBStringEncodingUTF8:
            codepoint = SBCodepointDecodeNextFromUTF8(codepointSequence->stringBuffer, codepointSequence->stringLength, stringIndex);
            break;

        case SBStringEncodingUTF16:
            codepoint = SBCodepointDecodeNextFromUTF16(codepointSequence->stringBuffer, codepointSequence->stringLength, stringIndex);
            break;

        case SBStringEncodingUTF32:
            codepoint = ((const SBUInt32 *) codepointSequence->stringBuffer)[(*stringIndex)++];
            if (!SBCodepointIsValid(codepoint)) codepoint = SBCodepointFaulty;
            break;
        }
    }

    return codepoint;
}
