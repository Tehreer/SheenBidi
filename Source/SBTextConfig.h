/*
 * Copyright (C) 2025 Muhammad Tayyab Akram
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

#ifndef _SB_INTERNAL_TEXT_CONFIG_H
#define _SB_INTERNAL_TEXT_CONFIG_H

#include <SheenBidi/SBAttributeRegistry.h>
#include <SheenBidi/SBBase.h>
#include <SheenBidi/SBTextConfig.h>

#include "Memory.h"
#include "Object.h"

typedef struct _AttributeArray {
    Memory _memory;
    const SBAttributeID *attributeIDs;
    const void * const *attributeValues;
    SBUInteger attributeCount;
} AttributeArray, *AttributeArrayRef;

typedef struct _SBTextConfig {
    ObjectBase _base;
    SBAttributeRegistryRef attributeRegistry;
    AttributeArray attributeArray;
    SBLevel baseLevel;
} SBTextConfig;

#endif
