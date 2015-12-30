/*
 * Copyright (C) 2014 Muhammad Tayyab Akram
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

#ifndef _SB_INTERNAL_RUN_EXTREMA_H
#define _SB_INTERNAL_RUN_EXTREMA_H

#include <SBTypes.h>

#include "SBCharType.h"

#define SB_RUN_EXTREMA                      SBUInt8

#define SB_RUN_EXTREMA__LEADING_L           (SB_CHAR_TYPE__L << 0)
#define SB_RUN_EXTREMA__LEADING_R           (SB_CHAR_TYPE__R << 0)

#define SB_RUN_EXTREMA__TRAILING_L          (SB_CHAR_TYPE__L << 4)
#define SB_RUN_EXTREMA__TRAILING_R          (SB_CHAR_TYPE__R << 4)

#define SB_RUN_EXTREMA__MAKE(sor, eor)      \
(SB_RUN_EXTREMA)                            \
(                                           \
   (sor << 0)                               \
 | (eor << 4)                               \
)

#define SB_RUN_EXTREMA__GET_SOR(e)          \
(                                           \
 (e) & 0xF                                  \
)

#define SB_RUN_EXTREMA__GET_EOR(e)          \
(                                           \
 (e) >> 4                                   \
)

enum {
    SBRunExtremaLeadingL  = SB_RUN_EXTREMA__LEADING_L,
    SBRunExtremaLeadingR  = SB_RUN_EXTREMA__LEADING_R,
    
    SBRunExtremaTrailingL = SB_RUN_EXTREMA__TRAILING_L,
    SBRunExtremaTrailingR = SB_RUN_EXTREMA__TRAILING_R
};
typedef SB_RUN_EXTREMA SBRunExtrema;

#endif
