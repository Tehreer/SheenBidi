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

#ifndef _SB_INTERNAL_ATTRIBUTE_DICTIONARY_H
#define _SB_INTERNAL_ATTRIBUTE_DICTIONARY_H

#include <SheenBidi/SBAttributeInfo.h>
#include <SheenBidi/SBAttributeRegistry.h>
#include <SheenBidi/SBConfig.h>
#include <SheenBidi/SBBase.h>
#include <SheenBidi/SBText.h>

#include "List.h"
#include "Object.h"

typedef LIST(SBAttributeItem) AttributeItemList, *AttributeItemListRef;

typedef struct _AttributeDictionary {
    ObjectBase _base;
    SBAttributeRegistryRef _registry;
    AttributeItemList _list;
} AttributeDictionary, *AttributeDictionaryRef;

/**
 * Adds or replaces an attribute item in an attribute item list.
 *
 * This function searches for an existing item with the same attribute ID in the list. If found, it
 * replaces the value (properly releasing the old value and retaining the new one). If not found, it
 * appends the new item to the list after retaining.
 *
 * @param itemList
 *      The attribute item list to modify.
 * @param registry
 *      The attribute registry for managing attribute values.
 * @param newItem
 *      The attribute item to add or replace.
 * @return
 *      SBTrue on success, SBFalse on failure (e.g., allocation failure).
 */
SB_INTERNAL SBBoolean AttributeItemListAdd(AttributeItemListRef itemList,
    SBAttributeRegistryRef registry, const SBAttributeItem *newItem);

/**
 * Creates a new attribute dictionary with the specified registry.
 *
 * Allocates and initializes a new attribute dictionary that uses the provided registry for managing
 * attribute lifecycle.
 *
 * @param registry
 *      The attribute registry that manages attribute retention and release.
 * @return
 *      A new attribute dictionary reference, or NULL if allocation fails.
 */
SB_INTERNAL AttributeDictionaryRef AttributeDictionaryCreate(SBAttributeRegistryRef registry);

SB_INTERNAL AttributeDictionaryRef AttributeDictionaryCopy(AttributeDictionaryRef dictionary);

/**
 * Checks whether an attribute dictionary is empty.
 *
 * @param dictionary
 *      The attribute dictionary to check.
 * @return
 *      SBTrue if the dictionary contains no attributes, SBFalse otherwise.
 */
SB_INTERNAL SBBoolean AttributeDictionaryIsEmpty(AttributeDictionaryRef dictionary);

/**
 * Retrieves filtered attribute items from a dictionary based on scope and group.
 *
 * @param dictionary
 *      The attribute dictionary to query. Can be NULL.
 * @param targetScope
 *      The attribute scope to filter by.
 * @param targetGroup
 *      The attribute group to filter by (or SBAttributeGroupNone for no group filtering).
 * @param resultList
 *      The list where matching items will be added.
 * @return
 *      SBTrue on success or if all comparisons match, SBFalse on failure or mismatch.
 */
SB_INTERNAL SBBoolean AttributeDictionaryFilter(AttributeDictionaryRef dictionary,
    SBAttributeScope targetScope, SBAttributeGroup targetGroup, AttributeItemListRef resultList);

/**
 * Searches for an attribute item with the specified ID in an attribute dictionary.
 *
 * @param dictionary
 *      The attribute dictionary to search. Can be NULL.
 * @param attributeID
 *      The attribute ID to search for.
 * @return
 *      A pointer to the matching attribute item, or NULL if not found or dictionary is NULL.
 */
SB_INTERNAL const SBAttributeItem *AttributeDictionaryFindItem(
    AttributeDictionaryRef dictionary, SBAttributeID attributeID);

/**
 * Checks if any attribute matching the specified scope and group exists in the dictionary.
 *
 * @param dictionary
 *      The attribute dictionary to check. Can be NULL.
 * @param targetScope
 *      The attribute scope to filter by.
 * @param targetGroup
 *      The attribute group to filter by (or SBAttributeGroupNone for no group filtering).
 * @return
 *      SBTrue if at least one matching attribute exists, SBFalse otherwise.
 */
SB_INTERNAL SBBoolean AttributeDictionaryMatchAny(AttributeDictionaryRef dictionary,
    SBAttributeScope targetScope, SBAttributeGroup targetGroup);

/**
 * Checks if all attributes matching scope and group have equal values between dictionary and list.
 *
 * This function compares attributes in the dictionary against those in the provided item list. It
 * filters both collections by the specified scope and group, then verifies that all filtered
 * attributes exist in both collections with equal values (using the registry's equality function).
 *
 * @param dictionary
 *      The attribute dictionary to check. Can be NULL (returns `SBTrue`).
 * @param targetScope
 *      The attribute scope to filter by (character, paragraph, or line).
 * @param targetGroup
 *      The attribute group to filter by, or SBAttributeGroupNone to match any group.
 * @param itemList
 *      The list of attribute items to compare against.
 * @return
 *      SBTrue if all matching attributes exist in both collections with equal values, SBFalse if
 *      any mismatch is found (missing or unequal attribute)
 */
SB_INTERNAL SBBoolean AttributeDictionaryMatchAll(AttributeDictionaryRef dictionary,
    SBAttributeScope targetScope, SBAttributeGroup targetGroup, AttributeItemListRef itemList);

/**
 * Removes an attribute with the specified ID from the dictionary.
 *
 * This function searches for the attribute by ID and removes it from the dictionary. The attribute
 * value is properly released through the registry before removal.
 *
 * @param dictionary
 *      The attribute dictionary from which to remove the attribute.
 * @param registry
 *      The registry used to release the attribute value.
 * @param attributeID
 *      The ID of the attribute to remove.
 */
SB_INTERNAL void AttributeDictionaryRemove(AttributeDictionaryRef dictionary,
    SBAttributeRegistryRef registry, SBAttributeID attributeID);

/**
 * Removes all attributes from the dictionary.
 *
 * This function iterates through all items in the dictionary and releases their values through the
 * registry before clearing the internal list.
 *
 * @param dictionary
 *      The attribute dictionary to clear.
 */
SB_INTERNAL void AttributeDictionaryClear(AttributeDictionaryRef dictionary);

/**
 * Increments the reference count of an attribute dictionary.
 *
 * @param dictionary
 *      The attribute dictionary to retain.
 * @return
 *      The same attribute dictionary passed in.
 */
SB_INTERNAL AttributeDictionaryRef AttributeDictionaryRetain(AttributeDictionaryRef dictionary);

/**
 * Decrements the reference count of an attribute dictionary.
 *
 * @param dictionary
 *      The attribute dictionary to release.
 *
 * @note
 *      When the reference count reaches zero, the dictionary is finalized and deallocated.
 */
SB_INTERNAL void AttributeDictionaryRelease(AttributeDictionaryRef dictionary);

#endif
