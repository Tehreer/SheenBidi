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

#ifndef _SB_PUBLIC_CODEPOINT_SEQUENCE_H
#define _SB_PUBLIC_CODEPOINT_SEQUENCE_H

#include "SBTypes.h"

SBCodepoint SBUnicodeGetCodepointForUTF8(SBUInt8 *buffer, SBUInteger length, SBUInteger *index, SBBoolean *isFaulty);

struct _SBCodepointSequence;
typedef struct _SBCodepointSequence SBCodepointSequence;
typedef SBCodepointSequence *SBCodepointSequenceRef;

typedef struct {
    SBUInteger bufferIndex;
    SBCodepoint codepoint;
    SBBoolean isFaulty;
} SBCodepointAgent;
typedef SBCodepointAgent *SBCodepointAgentRef;

SBCodepointSequenceRef SBCodepointSequenceCreate(void);

void SBCodepointSequenceLoadUTF8Buffer(SBCodepointSequenceRef codepointSequence, SBUInt8 *buffer, SBUInteger length);
void SBCodepointSequenceLoadUTF16Buffer(SBCodepointSequenceRef codepointSequence, SBUInt16 *buffer, SBUInteger length);
void SBCodepointSequenceLoadUTF32Buffer(SBCodepointSequenceRef codepointSequence, SBUInt32 *buffer, SBUInteger length);

SBCodepoint SBCodepointSequenceGetCodepointAt(SBCodepointSequenceRef codepointSequence, SBUInteger *bufferIndex);

SBCodepointSequenceRef SBCodepointSequenceRetain(SBCodepointSequenceRef codepointSequence);
void SBCodepointSequenceRelease(SBCodepointSequenceRef codepointSequence);

#endif
