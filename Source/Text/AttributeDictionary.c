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

#include <API/SBBase.h>

#if SB_TEXT_API_SUPPORTED

#include <stddef.h>

#include <API/SBAllocator.h>
#include <API/SBAttributeInfo.h>
#include <API/SBAttributeList.h>
#include <API/SBAttributeRegistry.h>
#include <Core/Object.h>

#include "AttributeDictionary.h"

/**
 * Determines whether an attribute item matches the specified filter criteria.
 *
 * Checks if the attribute's scope matches the target scope, unless the target scope is the
 * internal AttributeScopeAny sentinel, in which case every scope matches. If a specific group
 * filter is provided (not SBAttributeGroupNone), also verifies that the attribute's group
 * matches. Both conditions must be satisfied for the item to be considered a match.
 *
 * @param item
 *      The attribute item to check.
 * @param registry
 *      The attribute registry used to retrieve attribute metadata.
 * @param filterScope
 *      The scope to match against (character, paragraph, or the internal AttributeScopeAny).
 * @param filterGroup
 *      The group to match against, or SBAttributeGroupNone to skip group filtering.
 * @return
 *      SBTrue if the item matches all filter criteria, SBFalse otherwise.
 */
static SBBoolean CheckAttributeMatchesFilter(SBAttributeItem *item,
    SBAttributeRegistryRef registry, SBAttributeScope filterScope, SBAttributeGroup filterGroup)
{
    const SBAttributeInfo *attributeInfo;
    SBBoolean matchesFilter;

    attributeInfo = SBAttributeRegistryGetInfoReference(registry, item->attributeID);

    /* Match the scope first */
    matchesFilter = (filterScope == AttributeScopeAny || attributeInfo->scope == filterScope);

    /* Match the group if a specific group filter is provided */
    if (matchesFilter && filterGroup != SBAttributeGroupNone) {
        matchesFilter = (attributeInfo->group == filterGroup);
    }

    return matchesFilter;
}

SB_INTERNAL void AttributeDictionaryInitialize(AttributeDictionaryRef dictionary,
    SBAttributeRegistryRef registry)
{
    dictionary->_list = SBAttributeListCreate(registry);
}

SB_INTERNAL void AttributeDictionaryFinalize(AttributeDictionaryRef dictionary)
{
    SBAttributeListRelease(dictionary->_list);
}

SB_INTERNAL AttributeDictionaryRef AttributeDictionaryCreate(SBAttributeRegistryRef registry)
{
    AttributeDictionaryRef dictionary;

    dictionary = SBAllocatorAllocateBlock(NULL, sizeof(AttributeDictionary));

    if (dictionary) {
        AttributeDictionaryInitialize(dictionary, registry);
    }

    return dictionary;
}

SB_INTERNAL void AttributeDictionaryDestroy(AttributeDictionaryRef dictionary)
{
    AttributeDictionaryFinalize(dictionary);
    SBAllocatorDeallocateBlock(NULL, dictionary);
}

SB_INTERNAL SBBoolean AttributeDictionaryIsEmpty(AttributeDictionaryRef dictionary)
{
    return (SBAttributeListSize(dictionary->_list) == 0);
}

SB_INTERNAL void AttributeDictionarySet(AttributeDictionaryRef dictionary,
    AttributeDictionaryRef other)
{
    SBAttributeListRef otherList = other->_list;
    SBUInteger itemCount = SBAttributeListSize(otherList);
    SBUInteger itemIndex;

    AttributeDictionaryClear(dictionary);

    /* Copy each attribute item; the list retains a fresh reference to each value */
    for (itemIndex = 0; itemIndex < itemCount; itemIndex++) {
        const SBAttributeItem *source = SBAttributeListGetAt(otherList, itemIndex);
        const void *value = SBAttributeItemGetValuePtr(source);

        SBAttributeListInsertItem(dictionary->_list, itemIndex, source->attributeID, value);
    }
}

SB_INTERNAL void AttributeDictionaryPut(AttributeDictionaryRef dictionary,
    SBAttributeID attributeID, const void *attributeValue, SBBoolean *unchanged)
{
    SBAttributeListRef list = dictionary->_list;
    SBBoolean itemFound;
    SBUInteger itemIndex;

    /* Find the index where the item exists or should be inserted */
    itemIndex = SBAttributeListBinarySearchIndex(list, attributeID, &itemFound);

    if (itemFound) {
        if (unchanged) {
            SBAttributeItem *item = SBAttributeListGetAt(list, itemIndex);
            const void *previousValue = SBAttributeItemGetValuePtr(item);

            *unchanged = SBAttributeRegistryIsEqualAttribute(list->registry, attributeID,
                previousValue, attributeValue);
        }

        /* Replace with the new value; the list retains it and releases the previous one */
        SBAttributeListReplaceItemValue(list, itemIndex, attributeValue);
    } else {
        /* Insert the new item at the correct position to maintain sorted order */
        SBAttributeListInsertItem(list, itemIndex, attributeID, attributeValue);

        if (unchanged) {
            *unchanged = SBFalse;
        }
    }
}

SB_INTERNAL void AttributeDictionaryMerge(AttributeDictionaryRef dictionary,
    AttributeDictionaryRef other, SBBoolean *unchanged)
{
    SBUInteger itemCount = SBAttributeListSize(other->_list);
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
        SBAttributeItem *currentItem = SBAttributeListGetAt(other->_list, itemIndex);

        AttributeDictionaryPut(dictionary, currentItem->attributeID,
            SBAttributeItemGetValuePtr(currentItem), noImpact);

        if (noImpact && !remainedUnchanged) {
            *unchanged = SBFalse;
        }
    }
}

SB_INTERNAL void AttributeDictionaryFilter(AttributeDictionaryRef dictionary,
    SBAttributeScope targetScope, SBAttributeGroup targetGroup, AttributeDictionaryRef result)
{
    SBAttributeListRef list = dictionary->_list;
    SBAttributeRegistryRef registry = list->registry;
    SBUInteger itemCount = SBAttributeListSize(list);
    SBUInteger itemIndex;

    /* Clear the result dictionary before populating it */
    AttributeDictionaryClear(result);

    /* Iterate through all items in the dictionary */
    for (itemIndex = 0; itemIndex < itemCount; itemIndex++) {
        SBAttributeItem *currentItem = SBAttributeListGetAt(list, itemIndex);
        SBBoolean matched;

        /* Check if the item matches the filter criteria */
        matched = CheckAttributeMatchesFilter(currentItem, registry, targetScope, targetGroup);

        if (matched) {
            SBUInteger index =  SBAttributeListSize(result->_list);
            const void *value = SBAttributeItemGetValuePtr(currentItem);

            /* Append matching item to the result dictionary */
            SBAttributeListInsertItem(result->_list, index, currentItem->attributeID, value);
        }
    }
}

SB_INTERNAL const void *AttributeDictionaryFindValue(
    AttributeDictionaryRef dictionary, SBAttributeID attributeID)
{
    const void *value = NULL;
    SBUInteger itemIndex;
    SBBoolean itemFound;

    itemIndex = SBAttributeListBinarySearchIndex(dictionary->_list, attributeID, &itemFound);

    if (itemFound) {
        SBAttributeItem *item = SBAttributeListGetAt(dictionary->_list, itemIndex);
        value = SBAttributeItemGetValuePtr(item);
    }

    return value;
}

SB_INTERNAL SBBoolean AttributeDictionaryMatchAny(AttributeDictionaryRef dictionary,
    SBAttributeScope targetScope, SBAttributeGroup targetGroup)
{
    SBAttributeListRef list = dictionary->_list;
    SBAttributeRegistryRef registry = list->registry;
    SBUInteger itemCount = SBAttributeListSize(list);
    SBUInteger itemIndex;

    /* Iterate through all items */
    for (itemIndex = 0; itemIndex < itemCount; itemIndex++) {
        SBAttributeItem *currentItem = SBAttributeListGetAt(list, itemIndex);

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
    SBAttributeListRef list = dictionary->_list;
    SBAttributeRegistryRef registry = list->registry;
    SBUInteger dictCount = SBAttributeListSize(list);
    SBUInteger otherCount = SBAttributeListSize(other->_list);
    SBUInteger dictIndex;
    SBUInteger otherIndex;

    dictIndex = 0;
    otherIndex = 0;

    /* Compare filtered elements from both dictionaries */
    while (dictIndex < dictCount && otherIndex < otherCount) {
        SBAttributeItem *dictItem;
        SBAttributeItem *otherItem;

        dictItem = SBAttributeListGetAt(list, dictIndex);
        otherItem = SBAttributeListGetAt(other->_list, otherIndex);

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
            !SBAttributeRegistryIsEqualAttribute(registry, dictItem->attributeID,
                SBAttributeItemGetValuePtr(dictItem), SBAttributeItemGetValuePtr(otherItem))) {
            return SBFalse;
        }

        dictIndex += 1;
        otherIndex += 1;
    }

    /* Verify primary dictionary has no remaining filtered elements */
    while (dictIndex < dictCount) {
        SBAttributeItem *currentItem = SBAttributeListGetAt(dictionary->_list, dictIndex);

        if (CheckAttributeMatchesFilter(currentItem, registry, targetScope, targetGroup)) {
            return SBFalse;
        }

        dictIndex += 1;
    }

    /* Verify other dictionary has no remaining filtered elements */
    while (otherIndex < otherCount) {
        SBAttributeItem *currentItem = SBAttributeListGetAt(other->_list, otherIndex);

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

    itemIndex = SBAttributeListBinarySearchIndex(dictionary->_list, attributeID, &itemFound);

    if (itemFound) {
        SBAttributeListRemoveItem(dictionary->_list, itemIndex);
    }

    if (unchanged) {
        *unchanged = !itemFound;
    }
}

SB_INTERNAL void AttributeDictionaryClear(AttributeDictionaryRef dictionary)
{
    SBAttributeListClear(dictionary->_list);
}

SB_INTERNAL SBAttributeListRef AttributeDictionaryRelinquish(AttributeDictionaryRef dictionary)
{
    return SBAttributeListRetain(dictionary->_list);
}

#endif
