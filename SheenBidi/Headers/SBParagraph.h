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

#include "SBCodepointSequence.h"
#include "SBLine.h"
#include "SBTypes.h"

struct _SBParagraph;
typedef struct _SBParagraph SBParagraph;
typedef SBParagraph *SBParagraphRef;

/**
 * Provides the offset of the paragraph in original buffer.
 * @param paragraph
 *      The paragraph whose offset you want to obtain.
 * @return
 *      The offset of the paragraph passed in.
 */
SBUInteger SBParagraphGetOffset(SBParagraphRef paragraph);

/**
 * Provides the length of the paragraph in original buffer.
 * @param paragraph
 *      The paragraph whose length you want to obtain.
 * @return
 *      The length of the paragraph passed in.
 */
SBUInteger SBParagraphGetLength(SBParagraphRef paragraph);

/**
 * Provides the base level of the paragraph.
 * @param paragraph
 *      The paragraph whose base level you want to obtain.
 * @return
 *      The base level of the paragraph passed in.
 */
SBLevel SBParagraphGetBaseLevel(SBParagraphRef paragraph);

/**
 * Creates a line object by implementing rules L1 to L3 of unicode bidirectional algorithm.
 * @param paragraph
 *      The paragraph that creates the line.
 * @param lineOffset
 *      The absolute starting index of the line residing inside the paragraph.
 * @param lineLength
 *      The length of the line.
 * @return
 *      A reference to a line object if the call was successful, NULL otherwise.
 */
SBLineRef SBParagraphCreateLine(SBParagraphRef paragraph, SBUInteger lineOffset, SBUInteger lineLength);

SBParagraphRef SBParagraphRetain(SBParagraphRef paragraph);
void SBParagraphRelease(SBParagraphRef paragraph);

#endif
