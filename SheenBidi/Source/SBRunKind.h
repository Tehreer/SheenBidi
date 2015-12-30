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

#ifndef _SB_INTERNAL_RUN_KIND_H
#define _SB_INTERNAL_RUN_KIND_H

#include <SBTypes.h>

#define SB_RUN_KIND                         SBUInt8

#define SB_RUN_KIND__SIMPLE                 0x0

#define SB_RUN_KIND__ISOLATE                0x1
#define SB_RUN_KIND__PARTIAL                0x2
#define SB_RUN_KIND__PARTIAL_ISOLATE        \
(                                           \
   SB_RUN_KIND__ISOLATE                     \
 | SB_RUN_KIND__PARTIAL                     \
)

#define SB_RUN_KIND__TERMINATING            0x4
#define SB_RUN_KIND__ATTACHED               0x8
#define SB_RUN_KIND__ATTACHED_TERMINATING   \
(                                           \
   SB_RUN_KIND__TERMINATING                 \
 | SB_RUN_KIND__ATTACHED                    \
)

#define SB_RUN_KIND__MAKE(i, t)             \
(                                           \
   ((i) ? SB_RUN_KIND__PARTIAL_ISOLATE : 0) \
 | ((t) ? SB_RUN_KIND__TERMINATING : 0)     \
)

#define SB_RUN_KIND__MAKE_COMPLETE(t)       \
(                                           \
 (t) &= ~SB_RUN_KIND__PARTIAL               \
)

#define SB_RUN_KIND__MAKE_ATTACHED(t)       \
(                                           \
 (t) |= SB_RUN_KIND__ATTACHED               \
)

#define SB_RUN_KIND__IS_SIMPLE(t)           \
(                                           \
 (t) == SB_RUN_KIND__SIMPLE                 \
)

#define SB_RUN_KIND__IS_ISOLATE(t)          \
(                                           \
 (t) & SB_RUN_KIND__ISOLATE                 \
)

#define SB_RUN_KIND__IS_TERMINATING(t)      \
(                                           \
 (t) & SB_RUN_KIND__TERMINATING             \
)

#define SB_RUN_KIND__IS_PARTIAL_ISOLATE(t)  \
(                                           \
 ((t) & SB_RUN_KIND__PARTIAL)               \
)

#define SB_RUN_KIND__IS_COMPLETE_ISOLATE(t) \
(                                           \
    ((t) & SB_RUN_KIND__PARTIAL_ISOLATE)    \
 == SB_RUN_KIND__ISOLATE                    \
)

#define SB_RUN_KIND__IS_ATTACHED_TERMINATING(t) \
(                                           \
 ((t) & SB_RUN_KIND__ATTACHED)              \
)

enum {
    SBRunKindSimple      = SB_RUN_KIND__SIMPLE,

    SBRunKindIsolate     = SB_RUN_KIND__ISOLATE,
    SBRunKindPartial     = SB_RUN_KIND__PARTIAL,
    
    SBRunKindTerminating = SB_RUN_KIND__TERMINATING,
    SBRunKindAttached    = SB_RUN_KIND__ATTACHED
};
typedef SB_RUN_KIND SBRunKind;

#endif
