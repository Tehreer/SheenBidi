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

#include <stddef.h>

#include "Memory.h"
#include "Object.h"
#include "SBAttributeRegistry.h"
#include "SBBase.h"
#include "SBTextConfig.h"

static void InitializeAttributeArray(AttributeArrayRef attributeArray)
{
    MemoryInitialize(&attributeArray->_memory);
    attributeArray->attributeIDs = NULL;
    attributeArray->attributeValues = NULL;
    attributeArray->attributeCount = 0;
}

static void FinalizeAttributeArray(AttributeArrayRef attributeArray,
    SBAttributeRegistryRef attributeRegistry)
{
    const SBAttributeID *attributeIDs = attributeArray->attributeIDs;
    const void *const *attributeValues = attributeArray->attributeValues;
    SBUInteger attributeCount = attributeArray->attributeCount;
    SBUInteger index;

    for (index = 0; index < attributeCount; index++) {
        SBAttributeInfo attributeInfo;
        
        if (SBAttributeRegistryGetAttributeInfo(attributeRegistry, attributeIDs[index], &attributeInfo)) {
            attributeInfo.callbacks.release(attributeValues[index]);
        }
    }

    MemoryFinalize(&attributeArray->_memory);
}

#define ATTRIBUTE_IDS       0
#define ATTRIBUTE_VALUES    1
#define COUNT               2

static SBBoolean SetAttributeArray(AttributeArrayRef attributeArray,
    SBAttributeRegistryRef attributeRegistry,
    const SBAttributeID *attributeIDs, const void * const *attributeValues,
    SBUInteger attributeCount)
{
    MemoryRef memory = &attributeArray->_memory;
    SBBoolean isSet = SBFalse;
    void *pointers[COUNT] = { NULL };
    SBUInteger sizes[COUNT];

    sizes[ATTRIBUTE_IDS]    = sizeof(SBAttributeID) * attributeCount;
    sizes[ATTRIBUTE_VALUES] = sizeof(void *) * attributeCount;

    FinalizeAttributeArray(attributeArray, attributeRegistry);

    if (MemoryAllocateChunks(memory, MemoryTypePermanent, sizes, COUNT, pointers)) {
        SBAttributeID *ids = pointers[ATTRIBUTE_IDS];
        const void **values = pointers[ATTRIBUTE_VALUES];
        SBUInteger count = 0;
        SBUInteger index;

        for (index = 0; index < attributeCount; index++) {
            SBAttributeInfo attributeInfo;

            if (SBAttributeRegistryGetAttributeInfo(attributeRegistry, attributeIDs[index], &attributeInfo)) {
                ids[index] = attributeIDs[index];
                values[index] = attributeInfo.callbacks.retain(attributeValues[index]);
                count += 1;
            }
        }

        attributeArray->attributeIDs = ids;
        attributeArray->attributeValues = values;
        attributeArray->attributeCount = count;

        isSet = SBTrue;
    }

    return isSet;
}

#undef ATTRIBUTE_IDS
#undef ATTRIBUTE_VALUES
#undef ATTRIBUTE_COUNT
#undef COUNT

static void FinalizeTextConfig(ObjectRef object)
{
    SBTextConfigRef config = object;

    if (config->attributeRegistry) {
        SBAttributeRegistryRelease(config->attributeRegistry);
    }
}

SBTextConfigRef SBTextConfigCreate(void)
{
    const SBUInteger size = sizeof(SBTextConfig);
    void *pointer = NULL;
    SBTextConfigRef config;

    config = ObjectCreate(&size, 1, &pointer, &FinalizeTextConfig);

    if (config) {
        config->attributeRegistry = NULL;
        config->baseLevel = SBLevelDefaultLTR;

        InitializeAttributeArray(&config->attributeArray);
    }

    return config;
}

void SBTextConfigSetAttributeRegistry(SBTextConfigRef config,
    SBAttributeRegistryRef attributeRegistry)
{
    if (config->attributeRegistry) {
        SBAttributeRegistryRelease(config->attributeRegistry);
        config->attributeRegistry = NULL;
    }

    if (attributeRegistry) {
        config->attributeRegistry = SBAttributeRegistryRetain(attributeRegistry);
    }
}

void SBTextConfigSetDefaultAttributes(SBTextConfigRef config,
    const SBAttributeID *attributeIDs, const void * const *attributeValues,
    SBUInteger attributeCount)
{
    SetAttributeArray(&config->attributeArray, config->attributeRegistry,
        attributeIDs, attributeValues, attributeCount);
}

void SBTextConfigSetBaseLevel(SBTextConfigRef config, SBLevel baseLevel)
{
    config->baseLevel = baseLevel;
}

SBTextConfigRef SBTextConfigRetain(SBTextConfigRef config)
{
    return ObjectRetain((ObjectRef)config);
}

void SBTextConfigRelease(SBTextConfigRef config)
{
    ObjectRelease((ObjectRef)config);
}
