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

#ifndef _SB_INTERNAL_ATTRIBUTE_DICTIONARY_H
#define _SB_INTERNAL_ATTRIBUTE_DICTIONARY_H

#include <API/SBBase.h>

#if SB_TEXT_API_SUPPORTED

#include <SheenBidi/SBAttributeRegistry.h>
#include <API/SBAttributeList.h>

/**
 * Filter-only sentinel scope value that matches attributes of any scope. Never a real attribute's
 * registered scope, and deliberately not part of the public `SBAttributeScope` enum, so callers
 * can only reach "any scope" filtering through `SBAttributeFilterMakeAny`.
 */
#define AttributeScopeAny  ((SBAttributeScope)0xFF)

typedef struct _AttributeDictionary {
    SBAttributeListRef _list;
} AttributeDictionary, *AttributeDictionaryRef;

/**
 * Initializes an attribute dictionary, allocating a fresh, independently-retained
 * `SBAttributeListRef` (via `SBAttributeListCreate`) to back its storage.
 *
 * @param dictionary
 *      The attribute dictionary to initialize.
 * @param registry
 *      The attribute registry bound to this dictionary's storage; used to retain and release
 *      attribute values, and remembered by the underlying list for its own lifetime.
 */
SB_INTERNAL void AttributeDictionaryInitialize(AttributeDictionaryRef dictionary,
    SBAttributeRegistryRef registry);

/**
 * Finalizes an attribute dictionary, releasing its own reference to the underlying attribute
 * list. If no other reference was taken (e.g. via `AttributeDictionaryRelinquish`), the list's
 * values are released and its storage is freed.
 *
 * @param dictionary
 *      The attribute dictionary to finalize.
 */
SB_INTERNAL void AttributeDictionaryFinalize(AttributeDictionaryRef dictionary);

/**
 * Creates a new attribute dictionary on the heap.
 *
 * @param registry
 *      The attribute registry bound to this dictionary's storage; see
 *      `AttributeDictionaryInitialize`.
 * @return
 *      A new attribute dictionary reference, or NULL if allocation fails.
 */
SB_INTERNAL AttributeDictionaryRef AttributeDictionaryCreate(SBAttributeRegistryRef registry);

/**
 * Finalizes and deallocates an attribute dictionary previously obtained from
 * `AttributeDictionaryCreate`.
 *
 * @param dictionary
 *      The attribute dictionary to deallocate.
 */
SB_INTERNAL void AttributeDictionaryDestroy(AttributeDictionaryRef dictionary);

#define AttributeDictionaryGetCount(dictionary)     \
    SBAttributeListSize((dictionary)->_list)

/**
 * Checks whether an attribute dictionary is empty.
 *
 * @param dictionary
 *      The attribute dictionary to check.
 * @return
 *      `SBTrue` if empty, `SBFalse` otherwise.
 */
SB_INTERNAL SBBoolean AttributeDictionaryIsEmpty(AttributeDictionaryRef dictionary);

/**
 * Clears this dictionary and copies all attributes from the source, retaining their values
 * through the source's registry (`other`'s underlying list's stored registry).
 *
 * @param dictionary
 *      The attribute dictionary to modify.
 * @param other
 *      The source dictionary to copy from.
 */
SB_INTERNAL void AttributeDictionarySet(AttributeDictionaryRef dictionary,
    AttributeDictionaryRef other);

/**
 * Adds or updates an attribute item in the dictionary, retaining the value through the
 * dictionary's own registry (its underlying list's stored registry).
 *
 * If the attribute exists, updates its value; otherwise inserts it in sorted order.
 *
 * @param dictionary
 *      The destination dictionary.
 * @param attributeID
 *      The attribute ID to add or update.
 * @param attributeValue
 *      The attribute value.
 * @param unchanged
 *      Optional output parameter. Set to SBTrue if the item already existed with an equal value,
 *      SBFalse if item was inserted or value was changed.
 */
SB_INTERNAL void AttributeDictionaryPut(AttributeDictionaryRef dictionary,
    SBAttributeID attributeID, const void *attributeValue, SBBoolean *unchanged);

/**
 * Merges all attributes from another dictionary into this dictionary, retaining values through
 * the destination dictionary's own registry.
 *
 * @param dictionary
 *      The destination dictionary.
 * @param other
 *      The source dictionary to merge from.
 * @param unchanged
 *      Optional output parameter. Set to SBTrue if all items already existed with equal values,
 *      SBFalse if any item was inserted or modified.
 */
SB_INTERNAL void AttributeDictionaryMerge(AttributeDictionaryRef dictionary,
    AttributeDictionaryRef other, SBBoolean *unchanged);

/**
 * Retrieves filtered attributes matching the specified scope and group. Values are shared with
 * `dictionary` (borrowed, not independently retained), matching this function's role as a
 * read-only view rather than a value transfer.
 *
 * @param dictionary
 *      The attribute dictionary to query.
 * @param targetScope
 *      The attribute scope to filter by (e.g., character or paragraph).
 * @param targetGroup
 *      The attribute group to filter by. If SBAttributeGroupNone is specified, all groups within
 *      the scope are included.
 * @param result
 *      The dictionary where matching items will be added. Will be empty if no attributes match.
 *      Always cleared at the start.
 */
SB_INTERNAL void AttributeDictionaryFilter(AttributeDictionaryRef dictionary,
    SBAttributeScope targetScope, SBAttributeGroup targetGroup, AttributeDictionaryRef result);

/**
 * Searches for an attribute value by ID.
 *
 * @param dictionary
 *      The attribute dictionary to search.
 * @param attributeID
 *      The attribute ID to search for.
 * @return
 *      A pointer to the matching attribute value, or NULL if not found.
 */
SB_INTERNAL const void *AttributeDictionaryFindValue(AttributeDictionaryRef dictionary,
    SBAttributeID attributeID);

/**
 * Checks if any attribute matching the specified scope and group exists in the dictionary.
 *
 * @param dictionary
 *      The attribute dictionary to check.
 * @param targetScope
 *      The attribute scope to filter by (e.g., character or paragraph).
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
 * Compares filtered attributes (by scope and group) for equality and verifies both dictionaries
 * have the same filtered content. Uses `dictionary`'s own registry for equality comparison.
 *
 * @param dictionary
 *      The attribute dictionary to check.
 * @param targetScope
 *      The attribute scope to filter by (e.g., character or paragraph).
 * @param targetGroup
 *      The attribute group to filter by, or SBAttributeGroupNone to include all groups.
 * @param other
 *      The other attribute dictionary to compare against.
 * @return
 *      SBTrue if all filtered attributes exist in both dictionaries with equal values and neither
 *      dictionary has extra filtered attributes, SBFalse otherwise.
 */
SB_INTERNAL SBBoolean AttributeDictionaryMatchAll(AttributeDictionaryRef dictionary,
    SBAttributeScope targetScope, SBAttributeGroup targetGroup, AttributeDictionaryRef other);

/**
 * Removes an attribute with the specified ID, releasing its value through the dictionary's own
 * registry.
 *
 * @param dictionary
 *      The attribute dictionary from which to remove the attribute.
 * @param attributeID
 *      The ID of the attribute to remove.
 * @param unchanged
 *      Optional output parameter. Set to SBTrue if the attribute was not found, SBFalse if it was
 *      removed.
 */
SB_INTERNAL void AttributeDictionaryRemove(AttributeDictionaryRef dictionary,
    SBAttributeID attributeID, SBBoolean *unchanged);

/**
 * Removes all attributes from the dictionary, releasing their values through the dictionary's own
 * registry.
 *
 * @param dictionary
 *      The attribute dictionary to clear.
 */
SB_INTERNAL void AttributeDictionaryClear(AttributeDictionaryRef dictionary);

/**
 * Returns a new, independently-retained reference to the dictionary's underlying attribute list.
 *
 * Since the dictionary's storage is itself an `SBAttributeListRef` object (allocated by
 * `AttributeDictionaryInitialize`/`Create`), this is simply an extra retain on that object, not a
 * copy or a transfer of ownership — `dictionary` is left fully valid and should still be finalized
 * or destroyed normally once it is no longer needed.
 *
 * @param dictionary
 *      The attribute dictionary whose underlying list should be retained.
 * @return
 *      A new, retained attribute list.
 */
SB_INTERNAL SBAttributeListRef AttributeDictionaryRelinquish(AttributeDictionaryRef dictionary);

#endif

#endif
