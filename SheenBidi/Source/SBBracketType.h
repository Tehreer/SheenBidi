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

#ifndef _SB_INTERNAL_BRACKET_TYPE_H
#define _SB_INTERNAL_BRACKET_TYPE_H

#include <SBTypes.h>

#define SB_BRACKET_TYPE                 SBUInt8
#define SB_BRACKET_TYPE__NONE           0x00
#define SB_BRACKET_TYPE__OPEN           0x40
#define SB_BRACKET_TYPE__CLOSE          0x80

#define SB_BRACKET_TYPE__PRIMARY_MASK   \
(                                       \
   SB_BRACKET_TYPE__OPEN                \
 | SB_BRACKET_TYPE__CLOSE               \
)

#define SB_BRACKET_TYPE__INVERSE_MASK   \
(                                       \
 ~SB_BRACKET_TYPE__PRIMARY_MASK         \
)

enum {
    SBBracketTypeNone  = SB_BRACKET_TYPE__NONE,
    SBBracketTypeOpen  = SB_BRACKET_TYPE__OPEN,     /**< Opening paired bracket. */
    SBBracketTypeClose = SB_BRACKET_TYPE__CLOSE     /**< Closing paired bracket. */
};
typedef SB_BRACKET_TYPE SBBracketType;

#endif
