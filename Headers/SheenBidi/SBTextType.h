/*
 * Copyright (C) 2025-2026 Muhammad Tayyab Akram
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

#ifndef _SHEENBIDI_TEXT_TYPE_H
#define _SHEENBIDI_TEXT_TYPE_H

#include <SheenBidi/SBBase.h>

#if SB_TEXT_API_SUPPORTED

SB_EXTERN_C_BEGIN

/**
 * Opaque reference to an immutable text object.
 *
 * All query functions, such as those retrieving code units, bidirectional types, or resolved
 * levels, accept `SBTextRef`. Since `SBMutableTextRef` implicitly converts to `SBTextRef`, these
 * functions also work on a mutable text object.
 */
typedef const struct _SBText *SBTextRef;

/**
 * Opaque reference to a mutable text object.
 *
 * In addition to everything a `SBTextRef` supports, a mutable text object can be edited, e.g. by
 * inserting, deleting, or replacing code units, and by applying attributes.
 */
typedef struct _SBText *SBMutableTextRef;

SB_EXTERN_C_END

#endif

#endif
