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

#ifndef SB_BASE_DIRECTION_H
#define SB_BASE_DIRECTION_H

#include "SBTypes.h"

enum {
    SBBaseDirectionAutoLTR = 0x2000,
    SBBaseDirectionAutoRTL = 0x4000,
    SBBaseDirectionLTR     = 0x6000,
    SBBaseDirectionRTL     = 0x8000,
};
typedef SBUInt32 SBBaseDirection;

#endif
