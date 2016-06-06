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

#include <SBConfig.h>
#include <stddef.h>
#include <stdlib.h>

#include "SBCharType.h"
#include "SBCharTypeLookup.h"
#include "SBCodepointSequence.h"
#include "SBLog.h"
#include "SBParagraph.h"
#include "SBAlgorithm.h"

static SBAlgorithmRef _SBAlgorithmAllocate(SBUInteger stringLength)
{
    const SBUInteger sizeAlgorithm = sizeof(SBAlgorithm);
    const SBUInteger sizeTypes     = sizeof(SBCharType) * stringLength;

    const SBUInteger sizeMemory    = sizeAlgorithm
                                   + sizeTypes;

    SBUInt8 *memory = (SBUInt8 *)malloc(sizeMemory);

    SBUInteger offset = 0;
    SBAlgorithmRef algorithm = (SBAlgorithmRef)(memory + offset);

    offset += sizeAlgorithm;
    algorithm->fixedTypes = (SBCharType *)(memory + offset);

    return algorithm;
}

static void _SBDetermineCharTypes(SBCodepointSequenceRef sequence, SBCharType *types)
{
    SBUInteger stringIndex = 0;
    SBUInteger firstIndex = 0;
    SBCodepoint codepoint;

    while ((codepoint = SBCodepointSequenceGetCodepointAt(sequence, &stringIndex)) != SBCodepointInvalid) {
        types[firstIndex] = SBCharTypeDetermine(codepoint);

        /* Subsequent code units get 'BN' type. */
        while (++firstIndex < stringIndex) {
            types[firstIndex] = SBCharTypeBN;
        }
    }
}

SBAlgorithmRef SBAlgorithmCreate(SBCodepointSequenceRef codepointSequence)
{
    SBUInteger stringLength = codepointSequence->stringLength;

    if (stringLength > 0) {
        SBAlgorithmRef algorithm;

        SB_LOG_BLOCK_OPENER("Algorithm Input");
        SB_LOG_STATEMENT("Codepoints", 1, SB_LOG_CODEPOINT_SEQUENCE(codepointSequence));
        SB_LOG_BLOCK_CLOSER();

        algorithm = _SBAlgorithmAllocate(stringLength);
        algorithm->codepointSequence = SBCodepointSequenceRetain(codepointSequence);
        algorithm->_retainCount = 1;

        _SBDetermineCharTypes(codepointSequence, algorithm->fixedTypes);

        SB_LOG_BLOCK_OPENER("Determined Types");
        SB_LOG_STATEMENT("Types",  1, SB_LOG_CHAR_TYPES_ARRAY(algorithm->fixedTypes, stringLength));
        SB_LOG_BLOCK_CLOSER();

        SB_LOG_BREAKER();
        
        return algorithm;
    }

    return NULL;
}

SB_INTERNAL SBUInteger SBAlgorithmDetermineSeparatorLength(SBAlgorithmRef algorithm, SBUInteger separatorIndex)
{
    SBCodepointSequenceRef codepointSequence = algorithm->codepointSequence;
    SBUInteger stringIndex = separatorIndex;
    SBCodepoint codepoint;
    SBUInteger separatorLength;

    codepoint = SBCodepointSequenceGetCodepointAt(codepointSequence, &stringIndex);
    separatorLength = stringIndex - separatorIndex;

    if (codepoint == '\r') {
        /* Don't break in between 'CR' and 'LF'. */
        if (stringIndex < codepointSequence->stringLength) {
            codepoint = SBCodepointSequenceGetCodepointAt(codepointSequence, &stringIndex);

            if (codepoint == '\n') {
                separatorLength = stringIndex - separatorIndex;
            }
        }
    }

    return separatorLength;
}

void SBAlgorithmGetParagraphBoundary(SBAlgorithmRef algorithm,
    SBUInteger paragraphOffset, SBUInteger suggestedLength,
    SBUInteger *acutalLength, SBUInteger *separatorLength)
{
    SBCodepointSequenceRef codepointSequence = algorithm->codepointSequence;
    SBCharType *charTypes = algorithm->fixedTypes;
    SBUInteger limitIndex;
    SBUInteger startIndex;

    SBUIntegerNormalizeRange(codepointSequence->stringLength, &paragraphOffset, &suggestedLength);
    limitIndex = paragraphOffset + suggestedLength;

    for (startIndex = paragraphOffset; startIndex < limitIndex; startIndex++) {
        SBCharType currentType = charTypes[startIndex];

        if (currentType == SBCharTypeB) {
            if (separatorLength) {
                *separatorLength = SBAlgorithmDetermineSeparatorLength(algorithm, startIndex);
            }
            break;
        }
    }

    if (acutalLength) {
        *acutalLength = startIndex - paragraphOffset;
    }
}

SBParagraphRef SBAlgorithmCreateParagraph(SBAlgorithmRef algorithm,
    SBUInteger paragraphOffset, SBUInteger suggestedLength, SBLevel baseLevel)
{
    SBCodepointSequenceRef codepointSequence = algorithm->codepointSequence;
    SBUInteger stringLength = codepointSequence->stringLength;

    SBUIntegerNormalizeRange(stringLength, &paragraphOffset, &suggestedLength);

    if (suggestedLength > 0) {
        return SBParagraphCreate(algorithm, paragraphOffset, suggestedLength, baseLevel);
    }

    return NULL;
}

SBAlgorithmRef SBAlgorithmRetain(SBAlgorithmRef algorithm)
{
    if (algorithm) {
        ++algorithm->_retainCount;
    }

    return algorithm;
}

void SBAlgorithmRelease(SBAlgorithmRef algorithm)
{
    if (algorithm && --algorithm->_retainCount == 0) {
        SBCodepointSequenceRelease(algorithm->codepointSequence);
        free(algorithm);
    }
}
