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

enum {
    SBStringEncodingUTF8 = 0,
    SBStringEncodingUTF16 = 1,
    SBStringEncodingUTF32 = 2
};
typedef SBUInt32 SBStringEncoding;

typedef struct _SBCodepointSequence {
    SBStringEncoding stringEncoding;
    void *stringBuffer;
    SBUInteger stringLength;
} SBCodepointSequence;

SBCodepoint SBCodepointSequenceGetCodepointBefore(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex);
SBCodepoint SBCodepointSequenceGetCodepointAt(const SBCodepointSequence *codepointSequence, SBUInteger *stringIndex);

#endif
