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

#ifndef _SB_PUBLIC_ALGORITHM_H
#define _SB_PUBLIC_ALGORITHM_H

#include "SBBase.h"
#include "SBCodepointSequence.h"
#include "SBParagraph.h"

struct _SBAlgorithm;
typedef struct _SBAlgorithm SBAlgorithm;
typedef SBAlgorithm *SBAlgorithmRef;

SBAlgorithmRef SBAlgorithmCreate(SBCodepointSequenceRef codepointSequence);

void SBAlgorithmDetermineParagraphBoundary(SBAlgorithmRef algorithm,
    SBUInteger paragraphOffset, SBUInteger suggestedLength,
    SBUInteger *acutalLength, SBUInteger *separatorLength);

/**
 * Creates a paragraph object by implementing rules P1 to I2 of unicode bidirectional algorithm.
 * @param algorithm
 *      The algorithm object containing code point sequence of desired paragraph.
 * @param paragraphOffset
 *      The buffer offset of the paragraph in code point sequence.
 * @param suggestedLength
 *      The suggested buffer length of the paragraph in code point sequence.
 * @param baseLevel
 *      The desired base level of the paragraph.
 * @return
 *      A reference to a paragraph object if the call was successful, NULL otherwise.
 */
SBParagraphRef SBAlgorithmCreateParagraph(SBAlgorithmRef algorithm,
    SBUInteger paragraphOffset, SBUInteger suggestedLength, SBLevel baseLevel);

SBAlgorithmRef SBAlgorithmRetain(SBAlgorithmRef algorithm);
void SBAlgorithmRelease(SBAlgorithmRef algorithm);

#endif
