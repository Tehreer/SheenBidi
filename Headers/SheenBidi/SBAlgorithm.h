/*
 * Copyright (C) 2016-2026 Muhammad Tayyab Akram
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

#ifndef _SHEENBIDI_ALGORITHM_H
#define _SHEENBIDI_ALGORITHM_H

#include <SheenBidi/SBBase.h>
#include <SheenBidi/SBBidiType.h>
#include <SheenBidi/SBCodepointSequence.h>
#include <SheenBidi/SBParagraph.h>

SB_EXTERN_C_BEGIN

/**
 * A reference to an immutable object representing the Unicode Bidirectional Algorithm applied to
 * a code point sequence.
 */
typedef const struct _SBAlgorithm *SBAlgorithmRef;

/**
 * Creates an algorithm object for the specified code point sequence. The source string inside the
 * code point sequence should not be freed until the algorithm object is in use.
 *
 * @param codepointSequence
 *      The code point sequence to apply the bidirectional algorithm on.
 * @return
 *      A reference to an algorithm object if `codepointSequence` is valid (non-null, with a
 *      recognized encoding, a non-null buffer, and a non-zero length), `NULL` otherwise.
 */
SB_PUBLIC SBAlgorithmRef SBAlgorithmCreate(const SBCodepointSequence *codepointSequence);

/**
 * Returns a direct pointer to the bidirectional types of the code units, stored in the algorithm
 * object.
 *
 * @param algorithm
 *      The algorithm object from which to access the bidirectional types of the code units.
 * @return
 *      A valid pointer to an array of `SBBidiType` values, whose length is equal to that of the
 *      source string buffer.
 */
SB_PUBLIC const SBBidiType *SBAlgorithmGetBidiTypesPtr(SBAlgorithmRef algorithm);

/**
 * Determines the boundary of the first paragraph within the specified range.
 *
 * The boundary of the paragraph occurs after a code point whose bidirectional type is Paragraph
 * Separator (B), or at `suggestedLength` if no such code point exists before it. The exception to
 * this rule is when a Carriage Return (CR) is followed by a Line Feed (LF). Both CR and LF are
 * paragraph separators, but in that case, the boundary of the paragraph is considered after the LF
 * code point.
 *
 * @param algorithm
 *      The algorithm object to use for determining the paragraph boundary.
 * @param paragraphOffset
 *      The index to the first code unit of the paragraph in the source string.
 * @param suggestedLength
 *      The number of code units covering the suggested length of the paragraph.
 * @param actualLength
 *      On output, the actual length of the first paragraph, including the paragraph separator,
 *      within the given range.
 * @param separatorLength
 *      On output, the length of the paragraph separator. This parameter can be set to `NULL` if
 *      not needed.
 */
SB_PUBLIC void SBAlgorithmGetParagraphBoundary(SBAlgorithmRef algorithm,
    SBUInteger paragraphOffset, SBUInteger suggestedLength,
    SBUInteger *actualLength, SBUInteger *separatorLength);

/**
 * Creates a paragraph object processed with the Unicode Bidirectional Algorithm.
 *
 * This function processes only the first paragraph starting at `paragraphOffset` with length less
 * than or equal to `suggestedLength`, in accordance with Rule P1 of the Unicode Bidirectional
 * Algorithm.
 *
 * The paragraph level is determined by applying Rules P2-P3 and embedding levels are resolved by
 * applying Rules X1-I2.
 *
 * @param algorithm
 *      The algorithm object to use for creating the desired paragraph.
 * @param paragraphOffset
 *      The index to the first code unit of the paragraph in the source string.
 * @param suggestedLength
 *      The number of code units covering the suggested length of the paragraph.
 * @param baseLevel
 *      The desired base level of the paragraph. Rules P2-P3 are ignored if it is neither
 *      `SBLevelDefaultLTR` nor `SBLevelDefaultRTL`.
 * @return
 *      A reference to a paragraph object if the call was successful, `NULL` otherwise.
 */
SB_PUBLIC SBParagraphRef SBAlgorithmCreateParagraph(SBAlgorithmRef algorithm,
    SBUInteger paragraphOffset, SBUInteger suggestedLength, SBLevel baseLevel);

/**
 * Increments the reference count of an algorithm object.
 *
 * @param algorithm
 *      The algorithm object whose reference count will be incremented.
 * @return
 *      The same algorithm object passed in as the parameter.
 */
SB_PUBLIC SBAlgorithmRef SBAlgorithmRetain(SBAlgorithmRef algorithm);

/**
 * Decrements the reference count of an algorithm object. The object will be deallocated when its
 * reference count reaches zero.
 *
 * @param algorithm
 *      The algorithm object whose reference count will be decremented.
 */
SB_PUBLIC void SBAlgorithmRelease(SBAlgorithmRef algorithm);

SB_EXTERN_C_END

#endif
