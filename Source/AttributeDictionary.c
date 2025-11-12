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

/* =========================================================================
 * Attribute Item List Implementation
 * ========================================================================= */

/**
 * Searches for an attribute item with the specified ID in an attribute item list.
 *
 * @param itemList
 *      The attribute item list to search.
 * @param attributeID
 *      The attribute ID to search for.
 * @return
 *      A pointer to the matching attribute item, or NULL if not found.
 *
 * @note
 *      This function performs a linear search through the list.
 */
static const SBAttributeItem *FindAttributeItemInList(
    AttributeItemListRef itemList, SBAttributeID attributeID)
{
    SBUInteger itemCount = itemList->count;
    SBUInteger itemIndex;

    /* Linear search for the attribute ID */
    for (itemIndex = 0; itemIndex < itemCount; itemIndex++) {
        const SBAttributeItem *currentItem = ListGetRef(itemList, itemIndex);

        if (currentItem->attributeID == attributeID) {
            return currentItem;
        }
    }

    return NULL;
}

SB_INTERNAL SBBoolean AttributeItemListAdd(AttributeItemListRef itemList,
    SBAttributeRegistryRef registry, const SBAttributeItem *newItem)
{
    SBUInteger itemCount = itemList->count;
    SBUInteger itemIndex;

    /* Replace the existing item if available */
    for (itemIndex = 0; itemIndex < itemCount; itemIndex++) {
        SBAttributeItem *existingItem = ListGetRef(itemList, itemIndex);

        if (existingItem->attributeID == newItem->attributeID) {
            const void *previousValue = existingItem->attributeValue;

            if (registry) {
                /* Retain the new value */
                existingItem->attributeValue = SBAttributeRegistryRetainAttribute(registry,
                    newItem->attributeID, newItem->attributeValue);
                /* Release the old value */
                SBAttributeRegistryReleaseAttribute(registry,
                    existingItem->attributeID, previousValue);
            }

            return SBTrue;
        }
    }

    if (registry) {
        SBAttributeRegistryRetainAttribute(registry, newItem->attributeID, newItem->attributeValue);
    }

    /* Add as a new item if not found */
    return ListAdd(itemList, newItem);
}

/* =========================================================================
 * Attribute Dictionary Implementation
 * ========================================================================= */

/**
 * Finalizer callback invoked when an attribute dictionary's reference count reaches zero.
 *
 * @param object
 *      The object being finalized (an AttributeDictionaryRef).
 *
 * @note
 *      This function releases all retained attribute values and cleans up internal storage.
 */
static void FinalizeAttributeDictionary(ObjectRef object)
{
    AttributeDictionaryRef dictionary = object;

    /* Release all attribute values through the registry */
    AttributeDictionaryClear(dictionary);
    /* Finalize the internal list structure */
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
        /* Initialize the registry reference */
        dictionary->_registry = registry;
        /* Initialize the list to hold attribute items */
        ListInitialize(&dictionary->_list, sizeof(SBAttributeItem));
    }

    return dictionary;
}

SB_INTERNAL AttributeDictionaryRef AttributeDictionaryCopy(AttributeDictionaryRef dictionary)
{
    AttributeDictionaryRef copy = AttributeDictionaryCreate(dictionary->_registry);

    if (copy) {
        SBAttributeRegistryRef registry = copy->_registry;
        SBUInteger itemCount = dictionary->_list.count;
        SBUInteger itemIndex;
        SBBoolean succeeded;

        succeeded = ListReserveRange(&copy->_list, 0, itemCount);
        if (succeeded) {
            for (itemIndex = 0; itemIndex < itemCount; itemIndex++) {
                const SBAttributeItem *source = ListGetRef(&dictionary->_list, itemIndex);
                SBAttributeItem *destination = ListGetRef(&copy->_list, itemIndex);

                destination->attributeID = source->attributeID;
                destination->attributeValue = SBAttributeRegistryRetainAttribute(registry,
                    source->attributeID, source->attributeValue);
            }
        }

        if (!succeeded) {
            AttributeDictionaryRelease(copy);
            copy = NULL;
        }
    }

    return copy;
}

SB_INTERNAL SBBoolean AttributeDictionaryIsEmpty(AttributeDictionaryRef dictionary)
{
    return (dictionary->_list.count == 0);
}

SB_INTERNAL SBBoolean AttributeDictionaryFilter(AttributeDictionaryRef dictionary,
    SBAttributeScope targetScope, SBAttributeGroup targetGroup, AttributeItemListRef resultList)
{
    SBBoolean succeeded = SBTrue;

    if (dictionary) {
        SBAttributeRegistryRef registry = dictionary->_registry;
        AttributeItemListRef itemList = &dictionary->_list;
        SBUInteger itemCount = itemList->count;
        SBUInteger itemIndex;

        /* Iterate through all items in the dictionary */
        for (itemIndex = 0; itemIndex < itemCount; itemIndex++) {
            SBBoolean itemMatched = SBFalse;
            const SBAttributeItem *currentItem;
            const SBAttributeInfo *attributeInfo;

            currentItem = ListGetRef(itemList, itemIndex);
            attributeInfo = SBAttributeRegistryGetInfoReference(registry, currentItem->attributeID);

            /* Match the scope first */
            itemMatched = (attributeInfo->scope == targetScope);
            /* Match the group if needed */
            if (itemMatched && targetGroup != SBAttributeGroupNone) {
                itemMatched = (attributeInfo->group == targetGroup);
            }

            if (itemMatched) {
                succeeded = AttributeItemListAdd(resultList, NULL, currentItem);
            }

            /* Early exit on failure or mismatch */
            if (!succeeded) {
                break;
            }
        }
    }

    return succeeded;
}

SB_INTERNAL const SBAttributeItem *AttributeDictionaryFindItem(
    AttributeDictionaryRef dictionary, SBAttributeID attributeID)
{
    const SBAttributeItem *item = NULL;

    if (dictionary) {
        item = FindAttributeItemInList(&dictionary->_list, attributeID);
    }

    return item;
}

SB_INTERNAL SBBoolean AttributeDictionaryMatchAny(AttributeDictionaryRef dictionary,
    SBAttributeScope targetScope, SBAttributeGroup targetGroup)
{
    if (dictionary) {
        SBAttributeRegistryRef registry = dictionary->_registry;
        AttributeItemListRef itemList = &dictionary->_list;
        SBUInteger itemCount = itemList->count;
        SBUInteger itemIndex;

        /* Iterate through all items */
        for (itemIndex = 0; itemIndex < itemCount; itemIndex++) {
            SBBoolean itemMatched = SBFalse;
            const SBAttributeItem *currentItem;
            const SBAttributeInfo *attributeInfo;

            currentItem = ListGetRef(itemList, itemIndex);
            attributeInfo = SBAttributeRegistryGetInfoReference(registry, currentItem->attributeID);

            /* Match the scope first */
            itemMatched = (attributeInfo->scope == targetScope);
            /* Match the group if needed */
            if (itemMatched && targetGroup != SBAttributeGroupNone) {
                itemMatched = (attributeInfo->group == targetGroup);
            }

            /* Return true if there is a match */
            if (itemMatched) {
                return SBTrue;
            }
        }
    }

    return SBFalse;
}

SB_INTERNAL SBBoolean AttributeDictionaryMatchAll(AttributeDictionaryRef dictionary,
    SBAttributeScope targetScope, SBAttributeGroup targetGroup, AttributeItemListRef itemList)
{
    SBUInteger matchCount = 0;

    if (dictionary) {
        SBAttributeRegistryRef registry = dictionary->_registry;
        AttributeItemListRef dictList = &dictionary->_list;
        AttributeItemListRef outerList;
        AttributeItemListRef innerList;
        SBUInteger outerIndex;

        if (itemList->count > dictList->count) {
            outerList = itemList;
            innerList = dictList;
        } else {
            outerList = dictList;
            innerList = itemList;
        }

        /* Iterate through all items in the outer list */
        for (outerIndex = 0; outerIndex < outerList->count; outerIndex++) {
            SBBoolean itemMatched = SBTrue;
            SBBoolean isSkippable = SBFalse;
            const SBAttributeItem *currentItem;
            const SBAttributeInfo *attributeInfo;

            currentItem = ListGetRef(outerList, outerIndex);
            attributeInfo = SBAttributeRegistryGetInfoReference(registry, currentItem->attributeID);

            /* Match the scope first */
            isSkippable = (attributeInfo->scope != targetScope);
            /* Match the group if needed */
            if (!isSkippable && targetGroup != SBAttributeGroupNone) {
                isSkippable = (attributeInfo->group != targetGroup);
            }

            if (!isSkippable) {
                const SBAttributeItem *itemToCompare;

                itemToCompare = FindAttributeItemInList(innerList, currentItem->attributeID);

                if (itemToCompare) {
                    itemMatched = SBAttributeRegistryIsEqualAttribute(registry,
                        currentItem->attributeID, currentItem->attributeValue,
                        itemToCompare->attributeValue);
                    if (itemMatched) {
                        matchCount += 1;
                    }
                } else {
                    itemMatched = SBFalse;
                }
            }

            if (!itemMatched) {
                break;
            }
        }
    }

    return (matchCount == itemList->count);
}

SB_INTERNAL void AttributeDictionaryRemove(AttributeDictionaryRef dictionary,
    SBAttributeRegistryRef registry, SBAttributeID attributeID)
{
    SBUInteger itemIndex;

    /* Linear search for the attribute ID */
    for (itemIndex = 0; itemIndex < dictionary->_list.count; itemIndex++) {
        const SBAttributeItem *currentItem = ListGetRef(&dictionary->_list, itemIndex);

        if (currentItem->attributeID == attributeID) {
            /* Release the attribute value through the registry */
            SBAttributeRegistryReleaseAttribute(registry,
                currentItem->attributeID, currentItem->attributeValue);
            /* Remove the item from the list */
            ListRemoveAt(&dictionary->_list, itemIndex);
            break;
        }
    }
}

SB_INTERNAL void AttributeDictionaryClear(AttributeDictionaryRef dictionary)
{
    SBAttributeRegistryRef registry = dictionary->_registry;
    SBUInteger itemIndex;

    /* Release all attribute values through the registry */
    for (itemIndex = 0; itemIndex < dictionary->_list.count; itemIndex++) {
        const SBAttributeItem *item = ListGetRef(&dictionary->_list, itemIndex);
        SBAttributeRegistryReleaseAttribute(registry, item->attributeID, item->attributeValue);
    }

    /* Remove all items from the list */
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
