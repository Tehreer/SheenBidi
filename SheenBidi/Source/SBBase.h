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

#ifndef _SB_INTERNAL_BASE_H
#define _SB_INTERNAL_BASE_H

#include <SBBase.h>
#include <SBConfig.h>

/**
 * A value that indicates an invalid unsigned index.
 */
#define SBInvalidIndex  (SBUInteger)(-1)

#define SBUInt8InRange(v, s, e)     \
(                                   \
    (SBUInt8)((v) - (s))            \
 <= (SBUInt8)((e) - (s))            \
)

#define SBUInt16InRange(v, s, e)    \
(                                   \
    (SBUInt16)((v) - (s))           \
 <= (SBUInt16)((e) - (s))           \
)

#define SBUInt32InRange(v, s, e)    \
(                                   \
    (SBUInt32)((v) - (s))           \
 <= (SBUInt32)((e) - (s))           \
)

SB_INTERNAL void SBUIntegerNormalizeRange(SBUInteger actualLength, SBUInteger *rangeOffset, SBUInteger *rangeLength);
SB_INTERNAL SBBoolean SBUIntegerVerifyRange(SBUInteger actualLength, SBUInteger rangeOffset, SBUInteger rangeLength);

#define SBCodepointMax              0x10FFFF

#define SBCodepointInRange(v, s, e) SBUInt32InRange(v, s, e)

#define SBCodepointIsSurrogate(c)   SBCodepointInRange(c, 0xD800, 0xDFFF)

#define SBCodepointIsValid(c)       (!SBCodepointIsSurrogate(c) && (c) <= SBCodepointMax)

#endif
