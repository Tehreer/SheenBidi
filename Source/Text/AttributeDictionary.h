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
#include <SheenBidi/SBBase.h>
#include <SheenBidi/SBConfig.h>
#include <SheenBidi/SBText.h>

#include <Core/List.h>

typedef struct _AttributeDictionary {
    LIST(SBAttributeItem) _list;
} AttributeDictionary, *AttributeDictionaryRef;

/**
 * Initializes an attribute dictionary.
 *
 * Sets up an already-allocated attribute dictionary structure by initializing its internal list.
 * The internal list is initialized to store attribute items in sorted order by attribute ID.
 *
 * @param dictionary
 *      The attribute dictionary to initialize.
 */
SB_INTERNAL void AttributeDictionaryInitialize(AttributeDictionaryRef dictionary);

/**
 * Finalizes an attribute dictionary and releases all resources.
 *
 * Releases all retained attribute values through the registry and cleans up the internal list
 * structure. After finalization, the dictionary should not be used.
 *
 * @param dictionary
 *      The attribute dictionary to finalize.
 * @param registry
 *      The attribute registry used to release attribute values.
 */
SB_INTERNAL void AttributeDictionaryFinalize(AttributeDictionaryRef dictionary,
    SBAttributeRegistryRef registry);

/**
 * Creates a new attribute dictionary.
 *
 * @return
 *      A new attribute dictionary reference, or NULL if allocation fails.
 */
SB_INTERNAL AttributeDictionaryRef AttributeDictionaryCreate(void);

/**
 * Finalizes and deallocates an attribute dictionary.
 *
 * Releases all attribute values through the registry and deallocates the dictionary.
 *
 * @param dictionary
 *      The attribute dictionary to release.
 * @param registry
 *      The attribute registry used to release attribute values.
 */
SB_INTERNAL void AttributeDictionaryDestroy(AttributeDictionaryRef dictionary,
    SBAttributeRegistryRef registry);

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
 * Replaces all attributes in the dictionary with a copy of another dictionary's attributes.
 *
 * First clears all existing attributes from the dictionary (releasing their values through the
 * registry), then reserves space for and copies all attributes from the other dictionary.
 * All attribute values from the source are retained through the registry.
 *
 * @param dictionary
 *      The attribute dictionary to modify.
 * @param other
 *      The source attribute dictionary to copy from.
 * @param registry
 *      The attribute registry used to retain and release attribute values.
 */
SB_INTERNAL void AttributeDictionarySet(AttributeDictionaryRef dictionary,
    AttributeDictionaryRef other, SBAttributeRegistryRef registry);

/**
 * Adds or updates an attribute item in the dictionary.
 *
 * Searches for an attribute with the same ID. If found, replaces its value with the new one,
 * releasing the old value through the registry. If not found, inserts the new item at the
 * appropriate position to maintain sorted order by attribute ID. The new attribute value is
 * retained through the registry upfront.
 *
 * @param dictionary
 *      The attribute dictionary to modify.
 * @param newItem
 *      Pointer to the attribute item to add or update.
 * @param registry
 *      The attribute registry used to retain and release attribute values.
 * @param unchanged
 *      Optional output parameter. Set to SBTrue if the item already existed with an equal value,
 *      SBFalse if item was inserted or value was changed. If NULL, this check is not performed.
 */
SB_INTERNAL void AttributeDictionaryPut(AttributeDictionaryRef dictionary,
    const SBAttributeItem *newItem, SBAttributeRegistryRef registry, SBBoolean *unchanged);

/**
 * Merges all attributes from another dictionary into this dictionary.
 *
 * Iterates through all attributes in the source dictionary and adds or updates each one using
 * AttributeDictionaryPut. Sets unchanged to SBTrue only if all items already existed with equal
 * values; if any item was inserted or had its value changed, unchanged is set to SBFalse.
 *
 * @param dictionary
 *      The attribute dictionary to modify.
 * @param other
 *      The source dictionary to merge from.
 * @param registry
 *      The attribute registry used to retain and release attribute values.
 * @param unchanged
 *      Optional output parameter. Set to SBTrue if all items already existed with equal values,
 *      SBFalse if any item was inserted or modified. If NULL, this check is not performed.
 */
SB_INTERNAL void AttributeDictionaryMerge(AttributeDictionaryRef dictionary,
    AttributeDictionaryRef other, SBAttributeRegistryRef registry, SBBoolean *unchanged);

/**
 * Retrieves filtered attribute items from a dictionary based on scope and group.
 *
 * Clears the result dictionary, then iterates through all attributes in the source dictionary and
 * copies those matching the specified scope and group filters into the result dictionary.
 * Both dictionaries must have the same registry for proper filtering.
 *
 * @param dictionary
 *      The attribute dictionary to query.
 * @param targetScope
 *      The attribute scope to filter by (e.g., character or paragraph).
 * @param targetGroup
 *      The attribute group to filter by. If SBAttributeGroupNone is specified, all groups within
 *      the scope are included.
 * @param registry
 *      The attribute registry used to retrieve attribute metadata for filtering.
 * @param result
 *      The dictionary where matching items will be added. Will be empty if no attributes match.
 *      Always cleared at the start.
 */
SB_INTERNAL void AttributeDictionaryFilter(AttributeDictionaryRef dictionary,
    SBAttributeScope targetScope, SBAttributeGroup targetGroup,
    SBAttributeRegistryRef registry, AttributeDictionaryRef result);

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
 * Iterates through all attributes in the dictionary and returns SBTrue as soon as a match is found
 * based on scope and group filters. If targetGroup is SBAttributeGroupNone, only the scope is
 * checked.
 *
 * @param dictionary
 *      The attribute dictionary to check.
 * @param targetScope
 *      The attribute scope to filter by (e.g., character or paragraph).
 * @param targetGroup
 *      The attribute group to filter by (or SBAttributeGroupNone for no group filtering).
 * @param registry
 *      The attribute registry used to retrieve attribute metadata for filtering.
 * @return
 *      SBTrue if at least one matching attribute exists, SBFalse otherwise.
 */
SB_INTERNAL SBBoolean AttributeDictionaryMatchAny(AttributeDictionaryRef dictionary,
    SBAttributeScope targetScope, SBAttributeGroup targetGroup, SBAttributeRegistryRef registry);

/**
 * Checks if all filtered attributes have equal values between two dictionaries.
 *
 * Compares attributes in the dictionary against those in the other dictionary by iterating through
 * both in order. Only attributes matching the specified scope and group filters are considered.
 * Verifies that all filtered attributes exist in both collections with identical IDs and equal
 * values (using the registry's equality function), and that neither dictionary has extra filtered
 * attributes.
 *
 * @param dictionary
 *      The attribute dictionary to check.
 * @param targetScope
 *      The attribute scope to filter by (e.g., character or paragraph).
 * @param targetGroup
 *      The attribute group to filter by, or SBAttributeGroupNone to include all groups.
 * @param registry
 *      The attribute registry used to retrieve attribute metadata and compare equality.
 * @param other
 *      The other attribute dictionary to compare against.
 * @return
 *      SBTrue if all filtered attributes exist in both dictionaries with equal values and neither
 *      dictionary has extra filtered attributes, SBFalse otherwise.
 */
SB_INTERNAL SBBoolean AttributeDictionaryMatchAll(AttributeDictionaryRef dictionary,
    SBAttributeScope targetScope, SBAttributeGroup targetGroup,
    SBAttributeRegistryRef registry, AttributeDictionaryRef other);

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
 * @param registry
 *      The attribute registry used to release the attribute value.
 * @param unchanged
 *      Optional output parameter. Set to SBTrue if the attribute was not found, SBFalse if it was
 *      removed. If NULL, this check is not performed.
 */
SB_INTERNAL void AttributeDictionaryRemove(AttributeDictionaryRef dictionary,
    SBAttributeID attributeID, SBAttributeRegistryRef registry, SBBoolean *unchanged);

/**
 * Removes all attributes from the dictionary.
 *
 * Releases all attribute values through the registry and removes all items from the internal list.
 *
 * @param dictionary
 *      The attribute dictionary to clear.
 * @param registry
 *      The attribute registry used to release attribute values.
 */
SB_INTERNAL void AttributeDictionaryClear(AttributeDictionaryRef dictionary,
    SBAttributeRegistryRef registry);

#endif
