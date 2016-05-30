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

#ifndef _SB_INTERNAL_CODEPOINT_SEQUENCE_H
#define _SB_INTERNAL_CODEPOINT_SEQUENCE_H

#include <SBConfig.h>
#include <SBCodepointSequence.h>

#include "SBBase.h"

enum {
    SBEncodingUnknown = 0,
    SBEncodingUTF8 = 1,
    SBEncodingUTF16 = 2,
    SBEncodingUTF32 = 3
};
typedef SBUInt32 SBEncoding;

struct _SBCodepointSequence {
    SBEncoding _encoding;
    void *buffer;
    SBUInteger length;
    SBUInteger _retainCount;
};

#endif
