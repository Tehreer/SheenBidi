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
    AttributeDictionary _tempDict;
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

/**
 * Copies all attributes from a source attribute manager to this manager.
 *
 * Creates deep copies of all attribute dictionaries from the source manager. All attribute values
 * are retained through the registry during the copy process.
 *
 * @param manager
 *      The attribute manager to copy attributes into.
 * @param source
 *      The source attribute manager to copy from.
 * @return
 *      SBTrue on success, SBFalse if memory allocation or attribute retention fails.
 *
 * @note
 *      If the manager has no registry, this function returns SBTrue immediately without copying.
 * @note
 *      Any existing attributes in the manager are preserved; this function appends to the list.
 */
SB_INTERNAL SBBoolean AttributeManagerCopyAttributes(AttributeManagerRef manager,
    const AttributeManager *source);

/**
 * Reserves space for a range of code units and propagates adjacent attributes.
 *
 * This function is called when text is inserted. It allocates space in the internal attribute
 * dictionary array for the new code units. If an adjacent code unit has attributes, those
 * attributes are copied to all newly reserved code units to maintain consistency.
 *
 * The adjacent code unit is determined as follows:
 * - If inserting at the beginning (index == 0), attributes are copied from the position after
 *   the insertion (at offset `length`).
 * - Otherwise, attributes are copied from the code unit immediately before the insertion point
 *   (at index `index - 1`).
 *
 * @param manager
 *      The attribute manager to modify.
 * @param index
 *      The insertion index where the range will be reserved.
 * @param length
 *      The number of code units to reserve space for.
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
 * removal causes two paragraphs to merge (detected by checking if the code units immediately
 * before and after the removal point now belong to the same paragraph), paragraph-scoped
 * attributes are propagated across the merge boundary to ensure consistency.
 *
 * When a paragraph merge is detected:
 * - Paragraph-scoped attributes from the code unit before the merge point are applied to the
 *   second half of the merged paragraph (after the merge point).
 * - Paragraph-scoped attributes from the code unit after the merge point are applied to the
 *   first half of the merged paragraph (before the merge point).
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
 * This function applies an attribute to all code units in the specified range. For
 * paragraph-scoped attributes, the range is automatically expanded to include the full extent of
 * any partially covered paragraphs at the boundaries, ensuring that paragraph attributes are
 * applied uniformly across entire paragraphs.
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
 *      SBTrue on success, SBFalse if dictionary allocation, attribute retrieval, or attribute
 *      addition fails.
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
 * @note
 *      Empty attribute dictionaries are cached for reuse after attribute removal.
 */
SB_INTERNAL void AttributeManagerRemoveAttribute(AttributeManagerRef manager,
    SBUInteger index, SBUInteger length, SBAttributeID attributeID);

/**
 * Retrieves the next contiguous run with uniform value for a specific attribute.
 *
 * This function finds the next "run" starting from the specified index, where a run is a maximal
 * contiguous range of code units that either all have the same value for the specified attribute,
 * or all lack the attribute entirely. The function updates runStart to point to the end of the
 * found run (exclusive).
 *
 * @param manager
 *      The attribute manager to query.
 * @param runStart
 *      Pointer to the starting index; on return, updated to the index immediately following the
 *      end of the run (or unchanged if no run was found).
 * @param rangeEnd
 *      The exclusive upper bound for the search (not included in any run).
 * @param attributeID
 *      The ID of the attribute to filter by.
 * @param output
 *      Dictionary to populate with the attribute item if present in the run; will be empty if the
 *      run represents absent attributes.
 * @return
 *      SBTrue if a run was found (and the range was advanced), SBFalse if runStart >= rangeEnd
 *      (no more runs).
 *
 * @note
 *      The output dictionary is always cleared at the start of the function.
 * @note
 *      If the first code unit has the attribute, output will contain that attribute item, and
 *      the run extends while the value remains equal (compared using the registry's equality
 *      function).
 * @note
 *      If the first code unit lacks the attribute, output will be empty, and the run extends
 *      while the attribute continues to be absent.
 * @note
 *      This function is useful for efficiently iterating through attribute changes.
 */
SB_INTERNAL SBBoolean AttributeManagerGetOnwardRunByFilteringID(AttributeManagerRef manager,
    SBUInteger *runStart, SBUInteger rangeEnd,
    SBAttributeID attributeID, AttributeDictionaryRef output);

/**
 * Retrieves the next contiguous run with uniform filtered attribute collection.
 *
 * This function finds the next "run" starting from the specified index, where a run is a maximal
 * contiguous range of code units that all have the same collection of attributes matching the
 * specified scope and group filters. The function updates runStart to point to the end of the
 * found run (exclusive).
 *
 * A run consists of code units where:
 * - All have the same filtered attributes (same IDs and equal values), OR
 * - All have no matching attributes.
 *
 * @param manager
 *      The attribute manager to query.
 * @param runStart
 *      Pointer to the starting index; on return, updated to the index immediately following the
 *      end of the run (or unchanged if no run was found).
 * @param rangeEnd
 *      The exclusive upper bound for the search (not included in any run).
 * @param filterScope
 *      The attribute scope to filter by (e.g., character or paragraph).
 * @param filterGroup
 *      The attribute group to filter by, or SBAttributeGroupNone to match all groups within the
 *      scope.
 * @param output
 *      Dictionary to populate with all matching attribute items from the run.
 * @return
 *      SBTrue if a run was found (and the range was advanced), SBFalse if runStart >= rangeEnd
 *      (no more runs).
 *
 * @note
 *      The output dictionary is always cleared at the start of the function.
 * @note
 *      If the first code unit has matching attributes, output will contain all of them, and
 *      the run extends while all subsequent code units have exactly the same matching attributes
 *      (same IDs and equal values).
 * @note
 *      If the first code unit has no matching attributes, output will be empty, and the run
 *      extends while no matching attributes appear.
 * @note
 *      This function is useful for efficiently processing multiple related attributes together,
 *      such as all paragraph-level formatting attributes.
 */
SB_INTERNAL SBBoolean AttributeManagerGetOnwardRunByFilteringCollection(AttributeManagerRef manager,
    SBUInteger *runStart, SBUInteger rangeEnd,
    SBAttributeScope filterScope, SBAttributeGroup filterGroup, AttributeDictionaryRef output);

#endif
