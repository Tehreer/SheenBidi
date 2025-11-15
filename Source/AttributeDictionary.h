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

typedef struct _AttributeDictionary {
    ObjectBase _base;
    SBAttributeRegistryRef _registry;
    LIST(SBAttributeItem) _list;
} AttributeDictionary, *AttributeDictionaryRef;

/**
 * Initializes an attribute dictionary with the specified registry.
 *
 * Sets up an already-allocated attribute dictionary structure by initializing its base object
 * and internal list. The dictionary will use the provided registry for managing attribute
 * lifecycle (retention and release).
 *
 * @param dictionary
 *      The attribute dictionary to initialize.
 * @param registry
 *      The attribute registry that manages attribute retention and release.
 */
SB_INTERNAL void AttributeDictionaryInitialize(AttributeDictionaryRef dictionary,
    SBAttributeRegistryRef registry);

/**
 * Finalizes an attribute dictionary and releases all resources.
 *
 * Releases all retained attribute values through the registry and cleans up the internal list
 * structure. After finalization, the dictionary should not be used.
 *
 * @param dictionary
 *      The attribute dictionary to finalize.
 */
SB_INTERNAL void AttributeDictionaryFinalize(AttributeDictionaryRef dictionary);

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

/**
 * Creates a copy of an attribute dictionary.
 *
 * Allocates and initializes a new attribute dictionary as a deep copy of the provided dictionary.
 * All attribute values are retained through the registry during the copy process.
 *
 * @param dictionary
 *      The attribute dictionary to copy.
 * @return
 *      A new attribute dictionary reference containing copies of all attributes, or NULL if
 *      allocation fails.
 */
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
 * Adds or updates an attribute item in the dictionary.
 *
 * Inserts a new attribute item or replaces an existing one with the same ID. The new attribute
 * value is retained through the registry, and any previous value is released before replacement.
 *
 * @param dictionary
 *      The attribute dictionary to modify.
 * @param newItem
 *      Pointer to the attribute item to add or update.
 * @return
 *      SBTrue on success, SBFalse if insertion fails due to memory allocation.
 */
SB_INTERNAL SBBoolean AttributeDictionaryPut(AttributeDictionaryRef dictionary,
    const SBAttributeItem *newItem);

/**
 * Retrieves filtered attribute items from a dictionary based on scope and group.
 *
 * Copies all attributes from the dictionary that match the specified scope and group into the
 * result dictionary. The result dictionary is cleared before filtering begins.
 *
 * @param dictionary
 *      The attribute dictionary to query.
 * @param targetScope
 *      The attribute scope to filter by.
 * @param targetGroup
 *      The attribute group to filter by (or SBAttributeGroupNone for no group filtering).
 * @param result
 *      The dictionary where matching items will be added.
 * @return
 *      SBTrue on success, SBFalse on failure during insertion.
 */
SB_INTERNAL SBBoolean AttributeDictionaryFilter(AttributeDictionaryRef dictionary,
    SBAttributeScope targetScope, SBAttributeGroup targetGroup, AttributeDictionaryRef result);

/**
 * Searches for an attribute item with the specified ID in an attribute dictionary.
 *
 * Performs a binary search to locate the attribute by ID using the internally maintained sorted
 * order.
 *
 * @param dictionary
 *      The attribute dictionary to search.
 * @param attributeID
 *      The attribute ID to search for.
 * @return
 *      A pointer to the matching attribute item, or NULL if not found.
 */
SB_INTERNAL const SBAttributeItem *AttributeDictionaryFindItem(
    AttributeDictionaryRef dictionary, SBAttributeID attributeID);

/**
 * Checks if any attribute matching the specified scope and group exists in the dictionary.
 *
 * Iterates through all attributes in the dictionary and returns true as soon as a match is found.
 *
 * @param dictionary
 *      The attribute dictionary to check.
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
 * Checks if all filtered attributes have equal values between two dictionaries.
 *
 * Compares attributes in the dictionary against those in the other dictionary. It filters both
 * collections by the specified scope and group, then verifies that all filtered attributes exist
 * in both collections with identical IDs and equal values (using the registry's equality function).
 * Both dictionaries must have matching sets of filtered attributes.
 *
 * @param dictionary
 *      The attribute dictionary to check.
 * @param targetScope
 *      The attribute scope to filter by (character or paragraph).
 * @param targetGroup
 *      The attribute group to filter by, or SBAttributeGroupNone to match any group.
 * @param other
 *      The other attribute dictionary to compare against.
 * @return
 *      SBTrue if all filtered attributes exist in both dictionaries with equal values and neither
 *      dictionary has extra filtered attributes, SBFalse otherwise.
 */
SB_INTERNAL SBBoolean AttributeDictionaryMatchAll(AttributeDictionaryRef dictionary,
    SBAttributeScope targetScope, SBAttributeGroup targetGroup, AttributeDictionaryRef other);

/**
 * Removes an attribute with the specified ID from the dictionary.
 *
 * Searches for the attribute by ID and removes it from the dictionary. The attribute value is
 * properly released through the registry before removal. Does nothing if the attribute is not
 * found.
 *
 * @param dictionary
 *      The attribute dictionary from which to remove the attribute.
 * @param attributeID
 *      The ID of the attribute to remove.
 */
SB_INTERNAL void AttributeDictionaryRemove(AttributeDictionaryRef dictionary,
    SBAttributeID attributeID);

/**
 * Removes all attributes from the dictionary.
 *
 * Iterates through all items in the dictionary and releases their values through the registry
 * before clearing the internal list.
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
