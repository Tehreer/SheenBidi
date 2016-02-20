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

#ifndef _SB_INTERNAL_RUN_KIND_H
#define _SB_INTERNAL_RUN_KIND_H

#include <SBTypes.h>

enum {
    SBRunKindSimple         = 0x00,

    SBRunKindIsolate        = 0x01,
    SBRunKindPartial        = 0x02,
    SBRunKindPartialIsolate = SBRunKindIsolate | SBRunKindPartial,

    SBRunKindTerminating    = 0x04,
    SBRunKindAttached       = 0x08
};
typedef SBUInt8 SBRunKind;

#define SBRunKindMake(i, t)                 \
(                                           \
   ((i) ? SBRunKindPartialIsolate : 0)      \
 | ((t) ? SBRunKindTerminating : 0)         \
)

#define SBRunKindMakeComplete(k)            \
(                                           \
 (k) &= ~SBRunKindPartial                   \
)

#define SBRunKindMakeAttached(k)            \
(                                           \
 (k) |= SBRunKindAttached                   \
)

#define SBRunKindIsSimple(k)                \
(                                           \
 (k) == SBRunKindSimple                     \
)

#define SBRunKindIsIsolate(k)               \
(                                           \
 (k) & SBRunKindIsolate                     \
)

#define SBRunKindIsTerminating(k)           \
(                                           \
 (k) & SBRunKindTerminating                 \
)

#define SBRunKindIsPartialIsolate(k)        \
(                                           \
 (k) & SBRunKindPartial                     \
)

#define SBRunKindIsCompleteIsolate(k)       \
(                                           \
    ((k) & SBRunKindPartialIsolate)         \
 == SBRunKindIsolate                        \
)

#define SBRunKindIsAttachedTerminating(k)   \
(                                           \
 (k) & SBRunKindAttached                    \
)

#endif
