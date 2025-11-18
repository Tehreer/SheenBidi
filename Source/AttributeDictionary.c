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

#include <SheenBidi/SBAttributeInfo.h>
#include <SheenBidi/SBConfig.h>

#include "List.h"
#include "Object.h"
#include "SBAttributeRegistry.h"
#include "SBBase.h"
#include "AttributeDictionary.h"

/**
 * Finds the index where an attribute with the specified ID is located or should be inserted.
 *
 * Performs a binary search on the sorted list of attributes to locate an item by ID. If the
 * attribute is found, itemFound is set to SBTrue and the index of the existing item is returned.
 * If not found, itemFound is set to SBFalse and the index where the item should be inserted
 * (to maintain sorted order) is returned.
 *
 * @param dictionary
 *      The attribute dictionary to search.
 * @param attributeID
 *      The attribute ID to search for.
 * @param itemFound
 *      Pointer to a boolean that will be set to SBTrue if the attribute is found, SBFalse
 *      otherwise.
 * @return
 *      The index of the found item, or the insertion index if not found.
 */
static SBUInteger FindAttributeItemIndex(
    AttributeDictionaryRef dictionary, SBAttributeID attributeID, SBBoolean *itemFound)
{
    const SBAttributeItem *items = dictionary->_list.items;

    *itemFound = SBFalse;

    if (items) {
        SBUInteger low = 0;
        SBUInteger high = dictionary->_list.count;

        while (low < high) {
            SBUInteger mid = low + (high - low) / 2;

            if (items[mid].attributeID == attributeID) {
                *itemFound = SBTrue;
                return mid;
            }

            if (items[mid].attributeID < attributeID) {
                low = mid + 1;
            } else {
                high = mid;
            }
        }

        return low;
    }

    return 0;
}

/**
 * Determines whether an attribute item matches the specified filter criteria.
 *
 * Checks if the attribute's scope matches the target scope. If a specific group filter is
 * provided (not SBAttributeGroupNone), also verifies that the attribute's group matches.
 * Both conditions must be satisfied for the item to be considered a match.
 *
 * @param item
 *      The attribute item to check.
 * @param registry
 *      The attribute registry used to retrieve attribute metadata.
 * @param filterScope
 *      The scope to match against (character or paragraph).
 * @param filterGroup
 *      The group to match against, or SBAttributeGroupNone to skip group filtering.
 * @return
 *      SBTrue if the item matches all filter criteria, SBFalse otherwise.
 */
static SBBoolean CheckAttributeMatchesFilter(const SBAttributeItem *item,
    SBAttributeRegistryRef registry, SBAttributeScope filterScope, SBAttributeGroup filterGroup)
{
    const SBAttributeInfo *attributeInfo;
    SBBoolean matchesFilter;

    attributeInfo = SBAttributeRegistryGetInfoReference(registry, item->attributeID);

    /* Match the scope first */
    matchesFilter = (attributeInfo->scope == filterScope);

    /* Match the group if a specific group filter is provided */
    if (matchesFilter && filterGroup != SBAttributeGroupNone) {
        matchesFilter = (attributeInfo->group == filterGroup);
    }

    return matchesFilter;
}

/**
 * Initializes the internal data structures of an attribute dictionary.
 *
 * @param dictionary
 *      The attribute dictionary to initialize.
 * @param registry
 *      The attribute registry that manages attribute retention and release.
 */
static void InitializeAttributeDictionary(AttributeDictionaryRef dictionary,
    SBAttributeRegistryRef registry)
{
    /* Initialize the registry reference */
    dictionary->_registry = registry;
    /* Initialize the list to hold attribute items in sorted order */
    ListInitialize(&dictionary->_list, sizeof(SBAttributeItem));
}

/**
 * Finalizer callback invoked when an attribute dictionary's reference count reaches zero.
 *
 * This callback is registered with the object management system during creation and is
 * automatically invoked when all references to the dictionary have been released.
 * It delegates to AttributeDictionaryFinalize() to clean up resources.
 * 
 * @param object
 *      The object being finalized (an AttributeDictionaryRef).
 */
static void FinalizeAttributeDictionary(ObjectRef object)
{
    AttributeDictionaryFinalize(object);
}

static void ReleaseAttributeItemRange(AttributeDictionaryRef dictionary,
    SBUInteger startIndex, SBUInteger endIndex)
{
    SBAttributeRegistryRef registry = dictionary->_registry;
    SBUInteger itemIndex;

    if (registry) {
        /* Release attribute values through the registry */
        for (itemIndex = startIndex; itemIndex < endIndex; itemIndex++) {
            const SBAttributeItem *item = ListGetRef(&dictionary->_list, itemIndex);
            SBAttributeRegistryReleaseAttribute(registry, item->attributeID, item->attributeValue);
        }
    }
}

SB_INTERNAL void AttributeDictionaryInitialize(AttributeDictionaryRef dictionary,
    SBAttributeRegistryRef registry)
{
    ObjectBaseInitialize(&dictionary->_base);
    InitializeAttributeDictionary(dictionary, registry);
}

SB_INTERNAL void AttributeDictionaryFinalize(AttributeDictionaryRef dictionary)
{
    ReleaseAttributeItemRange(dictionary, 0, dictionary->_list.count);
    ListFinalize(&dictionary->_list);
}

SB_INTERNAL AttributeDictionaryRef AttributeDictionaryCreate(SBAttributeRegistryRef registry)
{
    const SBUInteger size = sizeof(AttributeDictionary);
    void *pointer = NULL;
    AttributeDictionaryRef dictionary;

    /* Create the object with a finalizer callback */
    dictionary = ObjectCreate(&size, 1, &pointer, FinalizeAttributeDictionary);

    if (dictionary) {
        InitializeAttributeDictionary(dictionary, registry);
    }

    return dictionary;
}

SB_INTERNAL SBBoolean AttributeDictionaryIsEmpty(AttributeDictionaryRef dictionary)
{
    return (dictionary->_list.count == 0);
}

SB_INTERNAL void AttributeDictionarySet(AttributeDictionaryRef dictionary,
    AttributeDictionaryRef other)
{
    SBAttributeRegistryRef registry = dictionary->_registry;
    SBUInteger itemCount = other->_list.count;
    SBUInteger itemIndex;

    AttributeDictionaryClear(dictionary);
    ListReserveRange(&dictionary->_list, 0, itemCount);

    /* Copy each attribute item, retaining values through the registry */
    for (itemIndex = 0; itemIndex < itemCount; itemIndex++) {
        const SBAttributeItem *source = ListGetRef(&other->_list, itemIndex);
        SBAttributeItem *destination = ListGetRef(&dictionary->_list, itemIndex);

        destination->attributeID = source->attributeID;
        destination->attributeValue = SBAttributeRegistryRetainAttribute(registry,
            source->attributeID, source->attributeValue);
    }
}

SB_INTERNAL void AttributeDictionaryPut(AttributeDictionaryRef dictionary,
    const SBAttributeItem *newItem, SBBoolean *unchanged)
{
    SBAttributeRegistryRef registry = dictionary->_registry;
    SBBoolean itemFound;
    SBUInteger itemIndex;

    /* Find the index where the item exists or should be inserted */
    itemIndex = FindAttributeItemIndex(dictionary, newItem->attributeID, &itemFound);

    if (registry) {
        /* Retain the new attribute value upfront */
        SBAttributeRegistryRetainAttribute(registry, newItem->attributeID, newItem->attributeValue);
    }

    if (itemFound) {
        SBAttributeItem *existingItem = ListGetRef(&dictionary->_list, itemIndex);
        SBAttributeID attributeID = existingItem->attributeID;

        /* Verify the item at this index has the matching ID */
        if (attributeID == newItem->attributeID) {
            const void *previousValue = existingItem->attributeValue;

            if (registry) {
                if (unchanged) {
                    *unchanged = SBAttributeRegistryIsEqualAttribute(registry,
                        attributeID, previousValue, newItem->attributeValue);
                }

                /* Release the old attribute value being replaced */
                SBAttributeRegistryReleaseAttribute(registry, attributeID, previousValue);
            } else {
                if (unchanged) {
                    *unchanged = (previousValue == newItem->attributeValue);
                }
            }

            /* Update with the new value */
            existingItem->attributeValue = newItem->attributeValue;
        }
    } else {
        /* Insert the new item at the correct position to maintain sorted order */
        ListInsert(&dictionary->_list, itemIndex, newItem);

        if (unchanged) {
            *unchanged = SBFalse;
        }
    }
}

SB_INTERNAL void AttributeDictionaryMerge(AttributeDictionaryRef dictionary,
    AttributeDictionaryRef other, SBBoolean *unchanged)
{
    SBUInteger itemCount = other->_list.count;
    SBUInteger itemIndex;
    SBBoolean remainedUnchanged;
    SBBoolean *noImpact;

    if (unchanged) {
        *unchanged = SBTrue;
        noImpact = &remainedUnchanged;
    } else {
        noImpact = NULL;
    }

    for (itemIndex = 0; itemIndex < itemCount; itemIndex++) {
        const SBAttributeItem *currentItem = ListGetRef(&other->_list, itemIndex);

        AttributeDictionaryPut(dictionary, currentItem, noImpact);

        if (noImpact && !remainedUnchanged) {
            *unchanged = SBFalse;
        }
    }
}

SB_INTERNAL void AttributeDictionaryFilter(AttributeDictionaryRef dictionary,
    SBAttributeScope targetScope, SBAttributeGroup targetGroup, AttributeDictionaryRef result)
{
    SBAttributeRegistryRef registry = dictionary->_registry;
    SBUInteger itemCount = dictionary->_list.count;
    SBUInteger itemIndex;

    /* Clear the result dictionary before populating it */
    AttributeDictionaryClear(result);

    /* Iterate through all items in the dictionary */
    for (itemIndex = 0; itemIndex < itemCount; itemIndex++) {
        const SBAttributeItem *currentItem = ListGetRef(&dictionary->_list, itemIndex);
        SBBoolean matched;

        /* Check if the item matches the filter criteria */
        matched = CheckAttributeMatchesFilter(currentItem, registry, targetScope, targetGroup);

        if (matched) {
            /* Add matching item to the result dictionary */
            ListAdd(&result->_list, currentItem);
        }
    }
}

SB_INTERNAL const SBAttributeItem *AttributeDictionaryFindItem(
    AttributeDictionaryRef dictionary, SBAttributeID attributeID)
{
    const SBAttributeItem *item = NULL;
    SBUInteger itemIndex;
    SBBoolean itemFound;

    itemIndex = FindAttributeItemIndex(dictionary, attributeID, &itemFound);

    if (itemFound) {
        item = ListGetRef(&dictionary->_list, itemIndex);
    }

    return item;
}

SB_INTERNAL SBBoolean AttributeDictionaryMatchAny(AttributeDictionaryRef dictionary,
    SBAttributeScope targetScope, SBAttributeGroup targetGroup)
{
    SBAttributeRegistryRef registry = dictionary->_registry;
    SBUInteger itemCount = dictionary->_list.count;
    SBUInteger itemIndex;

    /* Iterate through all items */
    for (itemIndex = 0; itemIndex < itemCount; itemIndex++) {
        const SBAttributeItem *currentItem = ListGetRef(&dictionary->_list, itemIndex);

        /* Return true if there is a match */
        if (CheckAttributeMatchesFilter(currentItem, registry, targetScope, targetGroup)) {
            return SBTrue;
        }
    }

    return SBFalse;
}

SB_INTERNAL SBBoolean AttributeDictionaryMatchAll(AttributeDictionaryRef dictionary,
    SBAttributeScope targetScope, SBAttributeGroup targetGroup, AttributeDictionaryRef other)
{
    SBAttributeRegistryRef registry = dictionary->_registry;
    SBUInteger dictCount = dictionary->_list.count;
    SBUInteger otherCount = other->_list.count;
    SBUInteger dictIndex;
    SBUInteger otherIndex;

    dictIndex = 0;
    otherIndex = 0;

    /* Compare filtered elements from both dictionaries */
    while (dictIndex < dictCount && otherIndex < otherCount) {
        const SBAttributeItem *dictItem;
        const SBAttributeItem *otherItem;

        dictItem = ListGetRef(&dictionary->_list, dictIndex);
        otherItem = ListGetRef(&other->_list, otherIndex);

        /* Skip non-matching items in the primary dictionary */
        if (!CheckAttributeMatchesFilter(dictItem, registry, targetScope, targetGroup)) {
            dictIndex += 1;
            continue;
        }

        /* Skip non-matching items in the other dictionary */
        if (!CheckAttributeMatchesFilter(otherItem, registry, targetScope, targetGroup)) {
            otherIndex += 1;
            continue;
        }

        /* Both items match the filter; verify they have the same ID and equal values */
        if (dictItem->attributeID != otherItem->attributeID ||
            !SBAttributeRegistryIsEqualAttribute(registry,
                dictItem->attributeID, dictItem->attributeValue, otherItem->attributeValue)) {
            return SBFalse;
        }

        dictIndex += 1;
        otherIndex += 1;
    }

    /* Verify primary dictionary has no remaining filtered elements */
    while (dictIndex < dictCount) {
        const SBAttributeItem *currentItem = ListGetRef(&dictionary->_list, dictIndex);

        if (CheckAttributeMatchesFilter(currentItem, registry, targetScope, targetGroup)) {
            return SBFalse;
        }

        dictIndex += 1;
    }

    /* Verify other dictionary has no remaining filtered elements */
    while (otherIndex < otherCount) {
        const SBAttributeItem *currentItem = ListGetRef(&other->_list, otherIndex);

        if (CheckAttributeMatchesFilter(currentItem, registry, targetScope, targetGroup)) {
            return SBFalse;
        }

        otherIndex += 1;
    }

    return SBTrue;
}

SB_INTERNAL void AttributeDictionaryRemove(AttributeDictionaryRef dictionary,
    SBAttributeID attributeID, SBBoolean *unchanged)
{
    SBUInteger itemIndex;
    SBBoolean itemFound;

    itemIndex = FindAttributeItemIndex(dictionary, attributeID, &itemFound);

    if (itemFound) {
        SBAttributeRegistryRef registry = dictionary->_registry;
        const SBAttributeItem *item;

        item = ListGetRef(&dictionary->_list, itemIndex);

        if (registry) {
            /* Release the attribute value through the registry before removal */
            SBAttributeRegistryReleaseAttribute(registry, item->attributeID, item->attributeValue);
        }

        /* Remove the item from the list */
        ListRemoveAt(&dictionary->_list, itemIndex);
    }

    if (unchanged) {
        *unchanged = !itemFound;
    }
}

SB_INTERNAL void AttributeDictionaryClear(AttributeDictionaryRef dictionary)
{
    ReleaseAttributeItemRange(dictionary, 0, dictionary->_list.count);
    ListRemoveAll(&dictionary->_list);
}

SB_INTERNAL AttributeDictionaryRef AttributeDictionaryRetain(AttributeDictionaryRef dictionary)
{
    return ObjectRetain(dictionary);
}

SB_INTERNAL void AttributeDictionaryRelease(AttributeDictionaryRef dictionary)
{
    ObjectRelease(dictionary);
}
