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

#include "SBBase.h"

struct _SBCodepointSequence;
typedef struct _SBCodepointSequence SBCodepointSequence;
typedef SBCodepointSequence *SBCodepointSequenceRef;

SBCodepointSequenceRef SBCodepointSequenceCreateWithUTF8Buffer(SBUInt8 *buffer, SBUInteger length);
SBCodepointSequenceRef SBCodepointSequenceCreateWithUTF16Buffer(SBUInt16 *buffer, SBUInteger length);
SBCodepointSequenceRef SBCodepointSequenceCreateWithUTF32Buffer(SBUInt32 *buffer, SBUInteger length);

SBCodepoint SBCodepointSequenceGetCodepointAt(SBCodepointSequenceRef codepointSequence, SBUInteger *bufferIndex);

SBCodepointSequenceRef SBCodepointSequenceRetain(SBCodepointSequenceRef codepointSequence);
void SBCodepointSequenceRelease(SBCodepointSequenceRef codepointSequence);

#endif
