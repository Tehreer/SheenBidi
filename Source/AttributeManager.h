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

#ifndef _SB_INTERNAL_ATTRIBUTE_MANAGER_H
#define _SB_INTERNAL_ATTRIBUTE_MANAGER_H

#include <SheenBidi/SBAttributeInfo.h>
#include <SheenBidi/SBAttributeRegistry.h>
#include <SheenBidi/SBConfig.h>
#include <SheenBidi/SBBase.h>
#include <SheenBidi/SBText.h>

#include "AttributeDictionary.h"
#include "List.h"

typedef struct _AttributeDictionaryCache {
    LIST(AttributeDictionaryRef) _attributeDicts;
} AttributeDictionaryCache, *AttributeDictionaryCacheRef;

typedef struct _AttributeManager {
    SBTextRef parent;
    SBAttributeRegistryRef _registry;
    AttributeDictionaryCache _cache;
    AttributeItemList _tempList;
    LIST(AttributeDictionaryRef) attributeDicts;
} AttributeManager, *AttributeManagerRef;

/**
 * Initializes an attribute manager.
 *
 * This function prepares the attribute manager for use by initializing its internal data
 * structures. If a registry is provided, the manager will support attribute operations; otherwise,
 * it will be in a passive state where all attribute operations are no-ops.
 *
 * @param manager
 *      The attribute manager to initialize.
 * @param parent
 *      The parent text object that owns this manager.
 * @param registry
 *      The attribute registry for managing attribute retention and release, or NULL to disable
 *      attribute support.
 *
 * @note
 *      If registry is NULL, the manager will not allocate any resources and all subsequent
 *      attribute operations will safely return without performing any work.
 * @note
 *      The parent text object must remain valid for the lifetime of the manager.
 */
SB_INTERNAL void AttributeManagerInitialize(AttributeManagerRef manager,
    SBTextRef parent, SBAttributeRegistryRef registry);

/**
 * Finalizes an attribute manager and releases all resources.
 *
 * This function releases all attribute dictionaries managed by the manager, clears the dictionary
 * cache, and finalizes all internal data structures. After calling this function, the manager must
 * not be used until re-initialized.
 *
 * @param manager
 *      The attribute manager to finalize.
 *
 * @note
 *      If the manager was initialized with a NULL registry, this function safely returns without
 *      performing any operations.
 * @note
 *      All attribute values are properly released through the registry before the dictionaries are
 *      deallocated.
 */
SB_INTERNAL void AttributeManagerFinalize(AttributeManagerRef manager);

SB_INTERNAL SBBoolean AttributeManagerCopyAttributes(AttributeManagerRef manager,
    const AttributeManager *source);

/**
 * Reserves space for a range of code units and propagates adjacent attributes.
 *
 * This function is called when text is inserted. It allocates space in the internal attribute
 * dictionary array for the new code units. If an adjacent code unit (the one immediately before the
 * insertion point, or at index 0 if inserting at the beginning) has attributes, those attributes
 * are copied to all newly reserved code units.
 *
 * @param index
 *      The insertion index where the range will be reserved.
 * @param length
 *      The number of code units to reserve space for.
 * @param manager
 *      The attribute manager to modify.
 * @return
 *      SBTrue on success, SBFalse if memory allocation fails.
 *
 * @pre
 *      length must be greater than 0.
 * @note
 *      If the manager has no registry, this function returns SBTrue immediately.
 * @note
 *      This ensures that inserted text inherits attributes from the surrounding context, which is
 *      the expected behavior in most text editing scenarios.
 */
SB_INTERNAL SBBoolean AttributeManagerReserveRange(AttributeManagerRef manager,
    SBUInteger index, SBUInteger length);

/**
 * Removes a range of code units and adjusts paragraph-scoped attributes.
 *
 * This function is called when text is removed. It releases and attempts to cache all attribute
 * dictionaries in the specified range, then removes the range from the internal array. If the
 * removal causes two paragraphs to merge (detected by checking if the code units immediately before
 * and after the removal point now belong to the same paragraph), paragraph-scoped attributes are
 * propagated across the merge boundary.
 *
 * @param manager
 *      The attribute manager to modify.
 * @param index
 *      The starting index of the range to remove.
 * @param length
 *      The number of code units to remove.
 * @return
 *      SBTrue on success, SBFalse if paragraph attribute adjustment fails.
 *
 * @pre
 *      length must be greater than 0.
 * @note
 *      If the manager has no registry, this function returns SBTrue immediately.
 * @note
 *      Paragraph merging is detected by comparing paragraph references at the boundaries; if they
 *      point to the same paragraph, attributes are synchronized.
 */
SB_INTERNAL SBBoolean AttributeManagerRemoveRange(AttributeManagerRef manager,
    SBUInteger index, SBUInteger length);

/**
 * Sets an attribute over a range of code units with automatic scope expansion.
 *
 * This function applies an attribute to all code units in the specified range. For paragraph-scoped
 * attributes, the range is automatically expanded to include the full extent of any partially
 * covered paragraphs at the boundaries, ensuring that paragraph attributes are applied uniformly
 * across entire paragraphs.
 *
 * @param manager
 *      The attribute manager to modify.
 * @param index
 *      The starting index of the range.
 * @param length
 *      The number of code units in the range.
 * @param attributeID
 *      The ID of the attribute to set.
 * @param attributeValue
 *      The value of the attribute (will be retained via the registry).
 * @return
 *      SBTrue on success, SBFalse if dictionary allocation or attribute addition fails.
 *
 * @pre
 *      length must be greater than 0.
 * @note
 *      For character-scoped attributes, only the exact range is affected.
 * @note
 *      For paragraph-scoped attributes, boundary paragraphs are fully included even if only
 *      partially covered by the original range.
 * @note
 *      If the manager has no registry, this function returns SBTrue immediately.
 */
SB_INTERNAL SBBoolean AttributeManagerSetAttribute(AttributeManagerRef manager,
    SBUInteger index, SBUInteger length, SBAttributeID attributeID, const void *attributeValue);

/**
 * Removes an attribute from a range of code units with automatic scope shrinking.
 *
 * This function removes a specific attribute from all code units in the specified range. For
 * paragraph-scoped attributes, the range is automatically shrunk to exclude any partially covered
 * paragraphs at the boundaries, ensuring that paragraph attributes are only removed from fully
 * covered paragraphs.
 *
 * @param manager
 *      The attribute manager to modify.
 * @param index
 *      The starting index of the range.
 * @param length
 *      The number of code units in the range.
 * @param attributeID
 *      The ID of the attribute to remove.
 *
 * @pre
 *      length must be greater than 0.
 * @note
 *      For character-scoped attributes, only the exact range is affected.
 * @note
 *      For paragraph-scoped attributes, boundary paragraphs are excluded if not fully covered by
 *      the original range, preventing partial removal.
 * @note
 *      If the resulting range has zero length after shrinking, no operation is performed.
 * @note
 *      If the manager has no registry, this function returns immediately.
 */
SB_INTERNAL void AttributeManagerRemoveAttribute(AttributeManagerRef manager,
    SBUInteger index, SBUInteger length, SBAttributeID attributeID);

/**
 * Retrieves the next contiguous run with uniform value for a specific attribute.
 *
 * This function finds the next "run" starting from the specified index, where a run is a maximal
 * contiguous range of code units that either all have the same value for the specified attribute,
 * or all lack the attribute entirely. The function updates the start index to point to the end of
 * the found run.
 *
 * @param manager
 *      The attribute manager to query.
 * @param runStart
 *      Pointer to the starting index; on success, updated to the index immediately following the
 *      end of the run.
 * @param rangeEnd
 *      The exclusive upper bound for the search (not included in any run).
 * @param attributeID
 *      The ID of the attribute to filter by.
 * @param outputItems
 *      List to populate with the attribute item if present in the run; will be empty if the run
 *      represents absent attributes.
 * @return
 *      SBTrue if a run was found, SBFalse if runStart >= rangeEnd (no more runs).
 *
 * @note
 *      The outputItems list is always cleared at the start of the function.
 * @note
 *      If the first code unit has the attribute, outputItems will contain that attribute item, and
 *      the run extends while the value remains equal (compared using the registry's equality
 *      function).
 * @note
 *      If the first code unit lacks the attribute, outputItems will be empty, and the run extends
 *      while the attribute continues to be absent.
 * @note
 *      This function is useful for efficiently iterating through attribute changes.
 */
SB_INTERNAL SBBoolean AttributeManagerGetOnwardRunByFilteringID(AttributeManagerRef manager,
    SBUInteger *runStart, SBUInteger rangeEnd,
    SBAttributeID attributeID, AttributeItemListRef outputItems);

/**
 * Retrieves the next contiguous run with uniform filtered attribute collection.
 *
 * This function finds the next "run" starting from the specified index, where a run is a maximal
 * contiguous range of code units that all have the same collection of attributes matching the
 * specified scope and group filters. The function updates the start index to point to the end of
 * the found run.
 *
 * @param manager
 *      The attribute manager to query.
 * @param runStart
 *      Pointer to the starting index; on success, updated to the index immediately following the
 *      end of the run.
 * @param rangeEnd
 *      The exclusive upper bound for the search (not included in any run).
 * @param filterScope
 *      The attribute scope to filter by (e.g., character or paragraph).
 * @param filterGroup
 *      The attribute group to filter by, or SBAttributeGroupNone to match all groups within the
 *      scope.
 * @param outputItems
 *      List to populate with all matching attribute items from the run.
 * @return
 *      SBTrue if a run was found, SBFalse if runStart >= rangeEnd (no more runs).
 *
 * @note
 *      The outputItems list is always cleared at the start of the function.
 * @note
 *      If the first code unit has matching attributes, outputItems will contain all of them, and
 *      the run extends while all subsequent code units have exactly the same matching attributes
 *      (same IDs and equal values).
 * @note
 *      If the first code unit has no matching attributes, outputItems will be empty, and the run
 *      extends while no matching attributes appear.
 * @note
 *      This function is useful for efficiently processing multiple related attributes together,
 *      such as all paragraph-level formatting attributes.
 */
SB_INTERNAL SBBoolean AttributeManagerGetOnwardRunByFilteringCollection(AttributeManagerRef manager,
    SBUInteger *runStart, SBUInteger rangeEnd,
    SBAttributeScope filterScope, SBAttributeGroup filterGroup, AttributeItemListRef outputItems);

#endif
