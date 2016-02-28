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

#ifndef _SB_PUBLIC_PARAGRAPH_H
#define _SB_PUBLIC_PARAGRAPH_H

#include "SBBaseDirection.h"
#include "SBTypes.h"

struct _SBParagraph;
typedef struct _SBParagraph SBParagraph;
typedef SBParagraph *SBParagraphRef;

typedef enum {
    SBParagraphOptionsNone = 0,
    SBParagraphOptionsDefault = SBParagraphOptionsNone
} SBParagraphOptions;

/**
 * Creates a paragraph object which implements rules P2 to I2 of unicode bidirectional algorithm.
 * @param codepoints
 *      The unicode code points for which the paragraph will be created.
 * @param length
 *      The length of the code points defining the end of the paragraph.
 * @param direction
 *      The base direction on which paragraph will be based.
 * @param options
 *      Desired options for the paragraph.
 * @return
 *      A reference to a paragraph object if the call was successful, NULL otherwise.
 */
SBParagraphRef SBParagraphCreateWithCodepoints(SBCodepoint *codepoints, SBUInteger length, SBBaseDirection direction, SBParagraphOptions options);

/**
 * Provides the base level of the paragraph.
 * @param paragraph
 *      The paragraph whose base level you want to obtain.
 * @return
 *      The base level of the paragraph passed in.
 */
SBLevel SBParagraphGetBaseLevel(SBParagraphRef paragraph);

SBParagraphRef SBParagraphRetain(SBParagraphRef paragraph);
void SBParagraphRelease(SBParagraphRef paragraph);

#endif
