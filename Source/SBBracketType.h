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

#ifndef _SB_BRACKET_TYPE_H
#define _SB_BRACKET_TYPE_H

#include <SBTypes.h>

#define _SB_BRACKET_TYPE                SBUInt8
#define _SB_BRACKET_TYPE__NONE          0x00
#define _SB_BRACKET_TYPE__OPEN          0x40
#define _SB_BRACKET_TYPE__CLOSE         0x80

#define _SB_BRACKET_TYPE__PRIMARY_MASK  \
(                                       \
   _SB_BRACKET_TYPE__OPEN               \
 | _SB_BRACKET_TYPE__CLOSE              \
)

#define _SB_BRACKET_TYPE__INVERSE_MASK  \
(                                       \
 ~_SB_BRACKET_TYPE__PRIMARY_MASK        \
)

enum {
    _SBBracketTypeNone  = _SB_BRACKET_TYPE__NONE,
    _SBBracketTypeOpen  = _SB_BRACKET_TYPE__OPEN,   /**< Opening paired bracket. */
    _SBBracketTypeClose = _SB_BRACKET_TYPE__CLOSE   /**< Closing paired bracket. */
};
typedef _SB_BRACKET_TYPE _SBBracketType;

#endif
