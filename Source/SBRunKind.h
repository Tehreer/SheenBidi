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

#ifndef _SB_RUN_KIND_H
#define _SB_RUN_KIND_H

#include <SBTypes.h>

#define _SB_RUN_KIND                        SBUInt8

#define _SB_RUN_KIND__SIMPLE                0x0

#define _SB_RUN_KIND__ISOLATE               0x1
#define _SB_RUN_KIND__PARTIAL               0x2
#define _SB_RUN_KIND__PARTIAL_ISOLATE       \
(                                           \
   _SB_RUN_KIND__ISOLATE                    \
 | _SB_RUN_KIND__PARTIAL                    \
)

#define _SB_RUN_KIND__TERMINATING           0x4
#define _SB_RUN_KIND__ATTACHED              0x8
#define _SB_RUN_KIND__ATTACHED_TERMINATING  \
(                                           \
   _SB_RUN_KIND__TERMINATING                \
 | _SB_RUN_KIND__ATTACHED                   \
)

#define _SB_RUN_KIND__MAKE(i, t)            \
(                                           \
   ((i) ? _SB_RUN_KIND__PARTIAL_ISOLATE : 0)\
 | ((t) ? _SB_RUN_KIND__TERMINATING : 0)    \
)

#define _SB_RUN_KIND__MAKE_COMPLETE(t)      \
(                                           \
 (t) &= ~_SB_RUN_KIND__PARTIAL              \
)

#define _SB_RUN_KIND__MAKE_ATTACHED(t)      \
(                                           \
 (t) |= _SB_RUN_KIND__ATTACHED              \
)

#define _SB_RUN_KIND__IS_SIMPLE(t)          \
(                                           \
 (t) == _SB_RUN_KIND__SIMPLE                \
)

#define _SB_RUN_KIND__IS_ISOLATE(t)         \
(                                           \
 (t) & _SB_RUN_KIND__ISOLATE                \
)

#define _SB_RUN_KIND__IS_TERMINATING(t)     \
(                                           \
 (t) & _SB_RUN_KIND__TERMINATING            \
)

#define _SB_RUN_KIND__IS_PARTIAL_ISOLATE(t) \
(                                           \
 ((t) & _SB_RUN_KIND__PARTIAL)              \
)

#define _SB_RUN_KIND__IS_COMPLETE_ISOLATE(t)\
(                                           \
    ((t) & _SB_RUN_KIND__PARTIAL_ISOLATE)   \
 == _SB_RUN_KIND__ISOLATE                   \
)

#define _SB_RUN_KIND__IS_ATTACHED_TERMINATING(t) \
(                                           \
 ((t) & _SB_RUN_KIND__ATTACHED)             \
)

enum {
    _SBRunKindSimple      = _SB_RUN_KIND__SIMPLE,

    _SBRunKindIsolate     = _SB_RUN_KIND__ISOLATE,
    _SBRunKindPartial     = _SB_RUN_KIND__PARTIAL,
    
    _SBRunKindTerminating = _SB_RUN_KIND__TERMINATING,
    _SBRunKindAttached    = _SB_RUN_KIND__ATTACHED,
};
typedef _SB_RUN_KIND _SBRunKind;

#endif
