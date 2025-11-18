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

#include <SheenBidi/SBConfig.h>

#include "List.h"
#include "Object.h"
#include "SBAttributeRegistry.h"
#include "SBBase.h"
#include "SBText.h"
#include "AttributeManager.h"

/* =========================================================================
 * Attribute Dictionary Cache Implementation
 * ========================================================================= */

/**
 * Initializes an attribute dictionary cache.
 *
 * The cache stores reusable attribute dictionaries to reduce memory allocations
 * when attributes are frequently added and removed.
 */
static void InitializeAttributeDictionaryCache(AttributeDictionaryCacheRef cache)
{
    ListInitialize(&cache->_attributeDicts, sizeof(AttributeDictionaryRef));
}

/**
 * Finalizes an attribute dictionary cache and releases all cached dictionaries.
 */
static void FinalizeAttributeDictionaryCache(AttributeDictionaryCacheRef cache)
{
    SBUInteger dictCount = cache->_attributeDicts.count;
    SBUInteger dictIndex;

    /* Release all cached attribute dictionaries */
    for (dictIndex = 0; dictIndex < dictCount; dictIndex++) {
        AttributeDictionaryRef dictionary = ListGetVal(&cache->_attributeDicts, dictIndex);
        AttributeDictionaryRelease(dictionary);
    }

    /* Finalize the cache list */
    ListFinalize(&cache->_attributeDicts);
}

/**
 * Retrieves a cached attribute dictionary or creates a new one if the cache is empty.
 *
 * @param cache
 *      The cache to retrieve from.
 * @param registry
 *      The attribute registry for creating new dictionaries if needed.
 * @return
 *      An attribute dictionary (cleared and ready for use).
 */
static AttributeDictionaryRef AcquireAttributeDictionaryFromCache(AttributeDictionaryCacheRef cache,
    SBAttributeRegistryRef registry)
{
    AttributeDictionaryRef dictionary = NULL;
    SBUInteger dictCount = cache->_attributeDicts.count;

    if (dictCount == 0) {
        /* Create a new dictionary if the cache is empty */
        dictionary = AttributeDictionaryCreate(registry);
    } else {
        /* Reuse the last cached dictionary */
        dictionary = ListGetVal(&cache->_attributeDicts, dictCount - 1);
        ListRemoveAt(&cache->_attributeDicts, dictCount - 1);
    }

    return dictionary;
}

/**
 * Caches an attribute dictionary for later reuse if it has no external references.
 *
 * The dictionary is only cached if its retain count is 1, indicating it's not in use elsewhere. The
 * dictionary is cleared before caching and retained for storage.
 *
 * @param cache
 *      The cache to store the dictionary in.
 * @param dictionary
 *      The attribute dictionary to cache.
 */
static void StoreAttributeDictionaryInCache(AttributeDictionaryCacheRef cache,
    AttributeDictionaryRef dictionary)
{
    SBUInteger retainCount = ObjectGetRetainCount(dictionary);

    /* Only cache if this is the last reference */
    if (retainCount == 1) {
        AttributeDictionaryClear(dictionary);
        AttributeDictionaryRetain(dictionary);
        ListAdd(&cache->_attributeDicts, &dictionary);
    }
}

/* =========================================================================
 * Attribute Entry Implementation
 * ========================================================================= */

/**
 * Initializes an attribute entry with the specified index and attributes.
 */
static void InitializeAttributeEntry(AttributeEntry *entry,
    SBUInteger index, AttributeDictionaryRef attributes)
{
    entry->index = index;
    entry->attributes = attributes;
}

/**
 * Finalizes an attribute entry and releases its attribute dictionary.
 */
static void FinalizeAttributeEntry(AttributeEntry *entry)
{
    if (entry->attributes) {
        AttributeDictionaryRelease(entry->attributes);
    }
}

/* =========================================================================
 * Attribute Manager Implementation
 * ========================================================================= */

/**
 * Operation types for attribute modifications.
 */
enum {
    AttributeOperationApply = 1,    /**< Apply/merge new attributes. */
    AttributeOperationRemove = 2    /**< Remove specific attribute. */
};
typedef SBUInt8 AttributeOperationType;

/**
 * Determines which side of the split receives the modified attributes.
 */
enum {
    SplitUpdateTargetRight = 0,     /**< Right segment gets modified attributes. */
    SplitUpdateTargetLeft = 1       /**< Left segment gets modified attributes. */
};
typedef SBUInt8 SplitUpdateTarget;

/**
 * Parameters for attribute operations.
 *
 * Uses a union to store operation-specific parameters without memory overhead.
 */
typedef union _AttributeOperationParams {
    struct {
        AttributeDictionaryRef attributes;  /**< Attributes to merge in. */
    } apply;
    struct {
        SBAttributeID attributeID;          /**< Attribute ID to remove. */
    } remove;
} AttributeOperationParams;

/**
 * Expands the range start to include the beginning of the first paragraph.
 *
 * @param manager
 *      The attribute manager containing the text.
 * @param[in,out] rangeStart
 *      Pointer to the range start index to modify.
 */
static void ExpandRangeToIncludeFirstParagraph(AttributeManagerRef manager, SBUInteger *rangeStart)
{
    SBTextRef text = manager->parent;
    SBUInteger paragraphIndex;
    TextParagraphRef paragraph;

    paragraphIndex = SBTextGetCodeUnitParagraphIndex(text, *rangeStart);
    paragraph = ListGetRef(&text->paragraphs, paragraphIndex);

    /* Expand to paragraph start */
    *rangeStart = paragraph->index;
}

/**
 * Expands the range end to include the end of the last paragraph.
 *
 * @param manager
 *      The attribute manager containing the text.
 * @param[in,out] rangeEnd
 *      Pointer to the range end index to modify.
 */
static void ExpandRangeToIncludeLastParagraph(AttributeManagerRef manager, SBUInteger *rangeEnd)
{
    SBTextRef text = manager->parent;
    SBUInteger paragraphIndex;
    TextParagraphRef paragraph;

    paragraphIndex = SBTextGetCodeUnitParagraphIndex(text, *rangeEnd - 1);
    paragraph = ListGetRef(&text->paragraphs, paragraphIndex);

    /* Expand to paragraph end */
    *rangeEnd = paragraph->index + paragraph->length;
}

/**
 * Expands both range boundaries to fully include any partially covered paragraphs.
 *
 * @param manager
 *      The attribute manager containing the text.
 * @param[in,out] rangeStart
 *      Pointer to the range start index to modify.
 * @param[in,out] rangeEnd
 *      Pointer to the range end index to modify.
 */
static void ExpandRangeToIncludeBoundaryParagraphs(AttributeManagerRef manager,
    SBUInteger *rangeStart, SBUInteger *rangeEnd)
{
    ExpandRangeToIncludeFirstParagraph(manager, rangeStart);
    ExpandRangeToIncludeLastParagraph(manager, rangeEnd);
}

/**
 * Shrinks the range start to exclude the first paragraph if it's not fully covered.
 *
 * @param manager
 *      The attribute manager containing the text.
 * @param[in,out] rangeStart
 *      Pointer to the range start index to modify.
 * @param rangeEnd
 *      The range end index.
 */
static void ShrinkRangeToExcludeFirstParagraph(AttributeManagerRef manager,
    SBUInteger *rangeStart, SBUInteger rangeEnd)
{
    SBTextRef text = manager->parent;
    SBUInteger paragraphIndex;
    TextParagraphRef textParagraph;
    SBUInteger paragraphStart;
    SBUInteger paragraphEnd;

    paragraphIndex = SBTextGetCodeUnitParagraphIndex(text, *rangeStart);
    textParagraph = ListGetRef(&text->paragraphs, paragraphIndex);

    paragraphStart = textParagraph->index;
    paragraphEnd = paragraphStart + textParagraph->length;

    /* Exclude the paragraph if not fully covered */
    if (*rangeStart > paragraphStart) {
        *rangeStart = paragraphEnd;

        /* Clamp to rangeEnd */
        if (*rangeStart > rangeEnd) {
            *rangeStart = rangeEnd;
        }
    }
}

/**
 * Shrinks the range end to exclude the last paragraph if it's not fully covered.
 *
 * @param manager
 *      The attribute manager containing the text.
 * @param rangeStart
 *      The range start index.
 * @param[in,out] rangeEnd
 *      Pointer to the range end index to modify.
 */
static void ShrinkRangeToExcludeLastParagraph(AttributeManagerRef manager,
    SBUInteger rangeStart, SBUInteger *rangeEnd)
{
    SBTextRef text = manager->parent;
    SBUInteger paragraphIndex;
    TextParagraphRef textParagraph;
    SBUInteger paragraphStart;
    SBUInteger paragraphEnd;

    paragraphIndex = SBTextGetCodeUnitParagraphIndex(text, *rangeEnd - 1);
    textParagraph = ListGetRef(&text->paragraphs, paragraphIndex);

    paragraphStart = textParagraph->index;
    paragraphEnd = paragraphStart + textParagraph->length;

    /* Exclude paragraph if not fully covered */
    if (paragraphEnd < *rangeEnd) {
        *rangeEnd = paragraphStart;

        /* Clamp to rangeStart */
        if (*rangeEnd < rangeStart) {
            *rangeEnd = rangeStart;
        }
    }
}

/**
 * Shrinks a range to exclude boundary paragraphs that are not fully covered.
 *
 * @param manager
 *      The attribute manager containing the text.
 * @param[in,out] rangeStart
 *      Pointer to the range start index to modify.
 * @param[in,out] rangeEnd
 *      Pointer to the range end index to modify.
 */
static void ShrinkRangeToExcludeBoundaryParagraphs(AttributeManagerRef manager,
    SBUInteger *rangeStart, SBUInteger *rangeEnd)
{
    ShrinkRangeToExcludeFirstParagraph(manager, rangeStart, *rangeEnd);

    if (*rangeEnd > *rangeStart) {
        ShrinkRangeToExcludeLastParagraph(manager, *rangeStart, rangeEnd);
    }
}

/**
 * Calculates the end index (exclusive) of an attribute entry.
 *
 * For all entries except the last, the end is the start of the next entry.
 * For the last entry, the end is the total code unit count.
 *
 * @param manager
 *      The attribute manager.
 * @param entryIndex
 *      The index of the entry.
 * @return
 *      The exclusive end index for this entry.
 */
static SBUInteger GetAttributeEntryEndIndex(AttributeManagerRef manager, SBUInteger entryIndex)
{
    if (entryIndex < manager->_entries.count - 1) {
        const AttributeEntry *nextEntry = ListGetRef(&manager->_entries, entryIndex + 1);
        return nextEntry->index;
    }

    return manager->_codeUnitCount;
}

/**
 * Shifts all entry indices starting from a given position by a delta amount.
 *
 * @param manager
 *      The attribute manager.
 * @param entryIndex
 *      The starting entry index (this entry and all following are shifted).
 * @param indexDelta
 *      The amount to shift (positive for insertion, negative for removal).
 */
static void ShiftAttributeEntryRanges(AttributeManagerRef manager,
    SBUInteger entryIndex, SBInteger indexDelta) {
    while (entryIndex < manager->_entries.count) {
        AttributeEntry *currentEntry = ListGetRef(&manager->_entries, entryIndex);
        currentEntry->index += indexDelta;
        entryIndex += 1;
    }
}

/**
 * Applies an attribute operation to a dictionary.
 *
 * @param attributes
 *      The attribute dictionary to modify.
 * @param operation
 *      Type of operation to perform.
 * @param params
 *      Operation-specific parameters.
 * @param unchanged
 *      Optional output: set to SBTrue if no changes were made.
 */
static void ApplyOperationToAttributes(AttributeDictionaryRef attributes,
    AttributeOperationType operation, AttributeOperationParams params, SBBoolean *unchanged)
{
    switch (operation) {
    case AttributeOperationApply:
        AttributeDictionaryMerge(attributes, params.apply.attributes, unchanged);
        break;
    case AttributeOperationRemove:
        AttributeDictionaryRemove(attributes, params.remove.attributeID, unchanged);
        break;
    }
}

/**
 * Splits an attribute entry at specified index and applies operation to one side.
 *
 * @param manager
 *      The attribute manager containing the entries.
 * @param entryIndex
 *      Index of the entry to split.
 * @param splitIndex
 *      Code unit index where to split the entry.
 * @param operation
 *      Operation to apply after split.
 * @param params
 *      Operation parameters.
 * @param updateTarget
 *      Which side of split receives the modified attributes.
 * @return
 *      SBTrue if split was performed, SBFalse if no split needed.
 */
static SBBoolean SplitAttributesEntry(AttributeManagerRef manager,
    SBUInteger entryIndex, SBUInteger splitIndex, AttributeOperationType operation,
    AttributeOperationParams params, SplitUpdateTarget updateTarget)
{
    AttributeEntry firstEntry;
    SBUInteger entryEnd;

    firstEntry = ListGetVal(&manager->_entries, entryIndex);
    entryEnd = GetAttributeEntryEndIndex(manager, entryIndex);

    /* Only split if split index is strictly between entry boundaries */
    if (splitIndex > firstEntry.index && splitIndex < entryEnd) {
        SBBoolean unchanged = SBTrue;
        AttributeDictionaryRef modifiedAttributes;

        /* Create working copy of attributes for modification */
        modifiedAttributes = AcquireAttributeDictionaryFromCache(&manager->_cache, manager->_registry);
        AttributeDictionarySet(modifiedAttributes, firstEntry.attributes);

        /* Apply operation to determine if changes would occur */
        ApplyOperationToAttributes(modifiedAttributes, operation, params, &unchanged);

        if (unchanged) {
            /* Operation produces no change - no split needed, cache the copy */
            StoreAttributeDictionaryInCache(&manager->_cache, modifiedAttributes);
        } else {
            AttributeEntry newEntry;
            newEntry.index = splitIndex;

            if (updateTarget == SplitUpdateTargetRight) {
                /* Right side gets modified attributes, left side keeps original */
                newEntry.attributes = modifiedAttributes;
            } else {
                AttributeEntry *original = ListGetRef(&manager->_entries, entryIndex);

                /* Left side gets modified attributes, right side keeps original */
                newEntry.attributes = original->attributes;
                original->attributes = modifiedAttributes;
            }

            ListInsert(&manager->_entries, entryIndex + 1, &newEntry);

            return SBTrue;
        }
    }

    return SBFalse;
}

/**
 * Core implementation for applying attribute operations over a code unit range.
 *
 * Handles three cases:
 * 1. Operation covers entire entry - apply directly
 * 2. Operation is entirely within one entry - split into 3 entries
 * 3. Operation spans multiple entries - split at boundaries and apply to all
 * 
 * @param manager
 *      The attribute manager to modify.
 * @param index
 *      Starting code unit index of operation range.
 * @param length
 *      Number of code units in operation range.
 * @param operation
 *      Type of operation to perform.
 * @param params
 *      Operation-specific parameters.
 */
static void ApplyOperationOverRange(AttributeManagerRef manager,
    SBUInteger index, SBUInteger length,
    AttributeOperationType operation, AttributeOperationParams params)
{
    SBUInteger rangeStart = index;
    SBUInteger rangeEnd = index + length;
    SBUInteger entryIndex;
    AttributeEntry *entry;
    SBUInteger entryEnd;

    /* Locate the entry containing the range start */
    entry = AttributeManagerFindEntry(manager, rangeStart, &entryIndex);
    entryEnd = GetAttributeEntryEndIndex(manager, entryIndex);

    if (rangeStart == entry->index && rangeEnd == entryEnd) {
        /* CASE 1: Operation covers entire entry exactly */
        ApplyOperationToAttributes(entry->attributes, operation, params, NULL);
    } else if (rangeStart > entry->index && rangeEnd < entryEnd) {
        /* CASE 2: Operation is entirely within one entry - split into 3 */
        AttributeDictionaryRef modifiedAttributes;
        SBBoolean unchanged;

        modifiedAttributes = AcquireAttributeDictionaryFromCache(&manager->_cache, manager->_registry);
        AttributeDictionarySet(modifiedAttributes, entry->attributes);
        ApplyOperationToAttributes(modifiedAttributes, operation, params, &unchanged);

        if (unchanged) {
            /* No changes needed - don't split */
            StoreAttributeDictionaryInCache(&manager->_cache, modifiedAttributes);
        } else {
            AttributeDictionaryRef cloneAttributes;

            cloneAttributes = AcquireAttributeDictionaryFromCache(&manager->_cache, manager->_registry);
            AttributeDictionarySet(cloneAttributes, entry->attributes);

            /* Reserve space for 2 new entries */
            ListReserveRange(&manager->_entries, entryIndex + 1, 2);

            /* Middle entry: gets modified attributes */
            entry = ListGetRef(&manager->_entries, entryIndex + 1);
            entry->index = rangeStart;
            entry->attributes = modifiedAttributes;

            /* Right entry: keeps original attributes */
            entry = ListGetRef(&manager->_entries, entryIndex + 2);
            entry->index = rangeEnd;
            entry->attributes = cloneAttributes;
        }
    } else {
        /* CASE 3: Operation spans multiple entries - handle boundaries and interior */
        SBUInteger entryProcessed = SBFalse;

        /* Split at range start if needed */
        if (SplitAttributesEntry(manager, entryIndex, rangeStart,
                operation, params, SplitUpdateTargetRight)) {
            entryProcessed = SBTrue;    /* Split already applied operation */
            entryIndex += 1;            /* Move to new right-side entry */
        }

        /* Process all entries intersecting with the operation range */
        while (rangeStart < rangeEnd) {
            entry = ListGetRef(&manager->_entries, entryIndex);
            entryEnd = GetAttributeEntryEndIndex(manager, entryIndex);

            /* Split at range end if current entry extends beyond operation range */
            if (entryEnd > rangeEnd) {
                /*
                 * Split at range end - modified attributes go to left side (range interior) so the
                 * portion inside range gets the operation applied
                 */
                SplitAttributesEntry(manager, entryIndex, rangeEnd,
                    operation, params, SplitUpdateTargetLeft);
                /* Remaining portion beyond range is unmodified */
                break;
            }

            /* Apply operation to current entry if not already processed by split */
            if (!entryProcessed) {
                ApplyOperationToAttributes(entry->attributes, operation, params, NULL);
            }

            /* Advance to next entry */
            rangeStart = entryEnd;
            entryIndex += 1;
            entryProcessed = SBFalse;
        }
    }
}

/**
 * Applies a set of attributes over a range by merging them into existing attributes.
 */
static void ApplyAttributesOverRange(AttributeManagerRef manager,
    SBUInteger index, SBUInteger length, AttributeDictionaryRef attributes)
{
    AttributeOperationParams params;
    params.apply.attributes = attributes;

    ApplyOperationOverRange(manager, index, length, AttributeOperationApply, params);
}

/**
 * Removes a specific attribute from a range of code units.
 */
static void RemoveAttributeFromRange(AttributeManagerRef manager,
    SBUInteger index, SBUInteger length, SBAttributeID attributeID)
{
    AttributeOperationParams params;
    params.remove.attributeID = attributeID;

    ApplyOperationOverRange(manager, index, length, AttributeOperationRemove, params);
}

/**
 * Adjusts paragraph-scoped attributes after text has been removed.
 *
 * When text is removed, separate paragraphs may merge into one. This function ensures
 * paragraph-scoped attributes are consistent across the merged boundary by:
 * - Copying paragraph attributes from before the merge point to the second half
 * - Copying paragraph attributes from after the merge point to the first half
 *
 * @param manager
 *      The attribute manager to adjust.
 * @param mergePointIndex
 *      The code unit index where the merge occurred.
 */
static void AdjustParagraphAttributesAfterMerge(AttributeManagerRef manager,
    SBUInteger mergePointIndex)
{
    if (mergePointIndex > 0 && mergePointIndex < manager->_codeUnitCount) {
        SBUInteger precedingIndex = mergePointIndex - 1;
        SBBoolean paragraphsMerged;
        TextParagraphRef precedingParagraph;
        TextParagraphRef followingParagraph;

        SBTextGetBoundaryParagraphs(manager->parent, precedingIndex, mergePointIndex,
            &precedingParagraph, &followingParagraph);
        paragraphsMerged = (precedingParagraph && followingParagraph
            && precedingParagraph == followingParagraph);

        if (paragraphsMerged) {
            SBUInteger paragraphStart = precedingParagraph->index;
            SBUInteger paragraphEnd = paragraphStart + precedingParagraph->length;
            AttributeDictionaryRef paragraphAttributes;
            AttributeEntry *entry;

            paragraphAttributes = &manager->_tempDict;
            AttributeDictionaryClear(paragraphAttributes);

            /* Extract paragraph-scoped attributes from before the merge point */
            entry = AttributeManagerFindEntry(manager, precedingIndex, NULL);

            AttributeDictionaryFilter(entry->attributes,
                SBAttributeScopeParagraph, SBAttributeGroupNone, paragraphAttributes);

            if (!AttributeDictionaryIsEmpty(paragraphAttributes)) {
                /* Apply to the second half of the merged paragraph */
                ApplyAttributesOverRange(manager,
                    mergePointIndex, paragraphEnd - mergePointIndex, paragraphAttributes);
            }

            /* Extract paragraph-scoped attributes from after the merge point */
            entry = AttributeManagerFindEntry(manager, mergePointIndex, NULL);

            AttributeDictionaryFilter(entry->attributes,
                SBAttributeScopeParagraph, SBAttributeGroupNone, paragraphAttributes);

            if (!AttributeDictionaryIsEmpty(paragraphAttributes)) {
                /* Apply to the first half of the merged paragraph */
                ApplyAttributesOverRange(manager,
                    paragraphStart, mergePointIndex - paragraphStart, paragraphAttributes);
            }
        }
    }
}

/**
 * Inserts the first attribute entry at index 0 with an empty attribute dictionary.
 *
 * This is called during initialization to create a base entry covering the entire text.
 */
static void InsertFirstAttributeEntry(AttributeManagerRef manager)
{
    AttributeDictionaryRef attributes;
    AttributeEntry entry;

    attributes = AcquireAttributeDictionaryFromCache(&manager->_cache, manager->_registry);
    InitializeAttributeEntry(&entry, 0, attributes);
    ListAdd(&manager->_entries, &entry);
}

/**
 * Removes and caches a range of attribute entries.
 *
 * Attempts to cache each removed entry's dictionary for reuse, then removes all entries in the
 * specified range from the entries list.
 *
 * @param manager
 *      The attribute manager.
 * @param index
 *      Starting index of range to remove.
 * @param length
 *      Number of entries to remove.
 */
static void RemoveAtributeEntryRange(AttributeManagerRef manager,
    SBUInteger index, SBUInteger length)
{
    SBUInteger rangeEnd = index + length;
    SBUInteger entryIndex;

    for (entryIndex = index; entryIndex < rangeEnd; entryIndex++) {
        AttributeEntry *entry = ListGetRef(&manager->_entries, entryIndex);

        StoreAttributeDictionaryInCache(&manager->_cache, entry->attributes);
    }

    ListRemoveRange(&manager->_entries, index, length);
}

SB_INTERNAL void AttributeManagerInitialize(AttributeManagerRef manager,
    SBTextRef parent, SBAttributeRegistryRef registry)
{
    manager->parent = parent;
    manager->_registry = registry;
    manager->_codeUnitCount = 0;

    if (registry) {
        /* Initialize all structures only when a registry is provided */
        InitializeAttributeDictionaryCache(&manager->_cache);
        AttributeDictionaryInitialize(&manager->_tempDict, NULL);
        ListInitialize(&manager->_entries, sizeof(AttributeEntry));
        InsertFirstAttributeEntry(manager);
    }
}

SB_INTERNAL void AttributeManagerFinalize(AttributeManagerRef manager)
{
    if (manager->_registry) {
        SBUInteger entryCount = manager->_entries.count;
        SBUInteger entryIndex;

        AttributeDictionaryFinalize(&manager->_tempDict);
        FinalizeAttributeDictionaryCache(&manager->_cache);

        /* Finalize all entries */
        for (entryIndex = 0; entryIndex < entryCount; entryIndex++) {
            AttributeEntry *entry = ListGetRef(&manager->_entries, entryIndex);
            FinalizeAttributeEntry(entry);
        }

        ListFinalize(&manager->_entries);
    }
}

SB_INTERNAL void AttributeManagerCopyAttributes(AttributeManagerRef manager,
    const AttributeManager *source)
{
    if (manager->_registry) {
        SBUInteger entryCount = source->_entries.count;
        SBUInteger entryIndex;

        /* Clear existing entries and cache their dictionaries */
        RemoveAtributeEntryRange(manager, 0, manager->_entries.count);
        /* Reserve space for source entries */
        ListReserveRange(&manager->_entries, 0, entryCount);

        /* Deep copy each entry and its attributes */
        for (entryIndex = 0; entryIndex < entryCount; entryIndex++) {
            const AttributeEntry *sourceEntry = ListGetRef(&source->_entries, entryIndex);
            AttributeEntry *newEntry = ListGetRef(&manager->_entries, entryIndex);

            newEntry->index = sourceEntry->index;
            newEntry->attributes = AttributeDictionaryCopy(sourceEntry->attributes);
        }

        manager->_codeUnitCount = source->_codeUnitCount;
    }
}

SB_INTERNAL AttributeEntry *AttributeManagerFindEntry(AttributeManagerRef manager,
    SBUInteger codeUnitIndex, SBUInteger *entryIndex)
{
    AttributeEntry *entries = manager->_entries.items;
    SBUInteger count = manager->_entries.count;
    SBUInteger codeUnitCount = manager->_codeUnitCount;
    SBUInteger low;
    SBUInteger high;

    SBAssert(entries && count > 0);

    low = 0;
    high = count - 1;

    while (low <= high) {
        SBUInteger mid = low + (high - low) / 2;
        AttributeEntry *entry;
        SBUInteger runStart;
        SBUInteger runEnd;

        entry = &entries[mid];
        runStart = entry->index;
        runEnd = (mid < count - 1) ? entries[mid + 1].index : codeUnitCount;

        if (codeUnitIndex < runStart) {
            /* Target is before the current entry */
            high = mid - 1;
        } else if (codeUnitIndex >= runEnd) {
            /* Target is after the current entry */
            low = mid + 1;
        } else {
            /* Target is within the current entry */
            if (entryIndex) {
                *entryIndex = mid;
            }

            return entry;
        }
    }

    return NULL;
}

SB_INTERNAL void AttributeManagerReserveRange(AttributeManagerRef manager,
    SBUInteger index, SBUInteger length)
{
    SBAssert(length > 0);

    if (manager->_registry) {
        SBUInteger codeUnitIndex;

        /* Determine which code unit's attributes to extend */
        codeUnitIndex = (index == 0 ? 0 : index - 1);

        if (codeUnitIndex < manager->_codeUnitCount) {
            SBUInteger entryIndex = SBInvalidIndex;

            AttributeManagerFindEntry(manager, codeUnitIndex, &entryIndex);

            if (entryIndex != SBInvalidIndex) {
                ShiftAttributeEntryRanges(manager, entryIndex + 1, length);
            }
        }

        manager->_codeUnitCount += length;
    }
}

SB_INTERNAL void AttributeManagerRemoveRange(AttributeManagerRef manager,
    SBUInteger index, SBUInteger length)
{
    SBAttributeRegistryRef registry = manager->_registry;
    SBUInteger rangeStart = index;
    SBUInteger rangeEnd = rangeStart + length;

    if (registry) {
        SBUInteger entryCount = manager->_entries.count;
        SBUInteger entryIndex = 0;
        AttributeEntry *entry;
        SBUInteger removalStart;
        SBUInteger removalCount;

        SBAssert(index < manager->_codeUnitCount && length > 0 && rangeEnd <= manager->_codeUnitCount);

        /* Find first entry that might be affected by removal */
        entry = AttributeManagerFindEntry(manager, index, &entryIndex);

        /* Skip to first entry that's completely within removal range */
        if (entry->index < rangeStart) {
            entryIndex += 1;
        }

        removalStart = entryIndex;
        removalCount = 0;

        /* Count and process entries to remove */
        while (entryIndex < entryCount) {
            SBUInteger endIndex;

            entry = ListGetRef(&manager->_entries, entryIndex);
            endIndex = GetAttributeEntryEndIndex(manager, entryIndex);

            if (endIndex <= rangeEnd) {
                /* Entry is completely within removal range - mark for removal */
                StoreAttributeDictionaryInCache(&manager->_cache, entry->attributes);
                FinalizeAttributeEntry(entry);

                removalCount += 1;
            } else {
                /* Entry extends beyond removal range - adjust its index */
                if (entry->index >= rangeEnd) {
                    entry->index -= length;
                } else {
                    entry->index = rangeStart;
                }
                /* Shift all subsequent entries */
                ShiftAttributeEntryRanges(manager, entryIndex + 1, -length);
                break;
            }

            entryIndex += 1;
        }

        /* Remove all marked entries */
        RemoveAtributeEntryRange(manager, removalStart, removalCount);
        manager->_codeUnitCount -= length;

        if (manager->_entries.count == 0) {
            InsertFirstAttributeEntry(manager);
        }

        /* Adjust paragraph attributes if removal caused paragraph merge */
        AdjustParagraphAttributesAfterMerge(manager, rangeStart);
    }
}

SB_INTERNAL void AttributeManagerSetAttribute(AttributeManagerRef manager,
    SBUInteger index, SBUInteger length, SBAttributeID attributeID, const void *attributeValue)
{
    SBAttributeRegistryRef registry = manager->_registry;

    SBAssert(length > 0);

    if (registry) {
        const SBAttributeInfo *attributeInfo;

        attributeInfo = SBAttributeRegistryGetInfoReference(registry, attributeID);

        if (attributeInfo) {
            AttributeDictionaryRef attributes = &manager->_tempDict;
            SBAttributeItem attributeItem;

            attributeItem.attributeID = attributeID;
            attributeItem.attributeValue = attributeValue;

            /* Expand the range for paragraph-scoped attributes */
            if (attributeInfo->scope == SBAttributeScopeParagraph) {
                SBUInteger startIndex = index;
                SBUInteger endIndex = startIndex + length;

                ExpandRangeToIncludeBoundaryParagraphs(manager, &startIndex, &endIndex);

                index = startIndex;
                length = endIndex - startIndex;
            }

            /* Prepare single-item dictionary with the attribute */
            AttributeDictionaryClear(attributes);
            AttributeDictionaryPut(attributes, &attributeItem, NULL);

            /* Apply over the (possibly expanded) range */
            ApplyAttributesOverRange(manager, index, length, attributes);
        }
    }
}

SB_INTERNAL void AttributeManagerRemoveAttribute(AttributeManagerRef manager,
    SBUInteger index, SBUInteger length, SBAttributeID attributeID)
{
    SBAttributeRegistryRef registry = manager->_registry;

    SBAssert(length > 0);

    if (registry) {
        const SBAttributeInfo *attributeInfo;

        attributeInfo = SBAttributeRegistryGetInfoReference(registry, attributeID);

        /* Update the range in case of paragraph scope */
        if (attributeInfo->scope == SBAttributeScopeParagraph) {
            SBUInteger startIndex = index;
            SBUInteger endIndex = startIndex + length;

            ShrinkRangeToExcludeBoundaryParagraphs(manager, &startIndex, &endIndex);

            index = startIndex;
            length = endIndex - startIndex;
        }

        if (length > 0) {
            RemoveAttributeFromRange(manager, index, length, attributeID);
        }
    }
}

SB_INTERNAL SBBoolean AttributeManagerGetOnwardRunByFilteringID(AttributeManagerRef manager,
    SBUInteger *runStart, SBUInteger rangeEnd,
    SBAttributeID attributeID, AttributeDictionaryRef output)
{
    SBUInteger entryIndex;
    const AttributeEntry *entry;
    const SBAttributeItem *initialItem;

    /* Clear output dictionary before populating */
    AttributeDictionaryClear(output);

    /* Check for the possibility of a next run first */
    if (*runStart >= rangeEnd) {
        return SBFalse;
    }

    /* Get the first entry and look for the attribute in it */
    entry = AttributeManagerFindEntry(manager, *runStart, &entryIndex);
    initialItem = AttributeDictionaryFindItem(entry->attributes, attributeID);

    *runStart = GetAttributeEntryEndIndex(manager, entryIndex);
    entryIndex += 1;

    if (initialItem) {
        /* Put the initial item to the output dictionary */
        AttributeDictionaryPut(output, initialItem, NULL);

        /* Iterate while the attribute value remains the same */
        while (*runStart < rangeEnd) {
            SBBoolean valuesMatched = SBFalse;
            const SBAttributeItem *subsequentItem;

            entry = ListGetRef(&manager->_entries, entryIndex);
            subsequentItem = AttributeDictionaryFindItem(entry->attributes, attributeID);

            /* Check if the attribute value matches */
            if (subsequentItem) {
                valuesMatched = SBAttributeRegistryIsEqualAttribute(manager->_registry,
                    attributeID, initialItem->attributeValue, subsequentItem->attributeValue);
            }

            /* Stop if the value changes */
            if (!valuesMatched) {
                break;
            }

            *runStart = GetAttributeEntryEndIndex(manager, entryIndex);
            entryIndex += 1;
        }
    } else {
        /* Iterate while the attribute doesn't exist */
        while (*runStart < rangeEnd) {
            const SBAttributeItem *subsequentItem;

            entry = ListGetRef(&manager->_entries, entryIndex);
            subsequentItem = AttributeDictionaryFindItem(entry->attributes, attributeID);

            /* Stop when the attribute appears */
            if (subsequentItem) {
                break;
            }

            *runStart = GetAttributeEntryEndIndex(manager, entryIndex);
            entryIndex += 1;
        }
    }

    if (*runStart > rangeEnd) {
        *runStart = rangeEnd;
    }

    return SBTrue;
}

SB_INTERNAL SBBoolean AttributeManagerGetOnwardRunByFilteringCollection(AttributeManagerRef manager,
    SBUInteger *runStart, SBUInteger rangeEnd,
    SBAttributeScope filterScope, SBAttributeGroup filterGroup, AttributeDictionaryRef output)
{
    SBUInteger entryIndex;
    const AttributeEntry *entry;

    /* Clear the output dictionary before populating */
    AttributeDictionaryClear(output);

    /* Check for the possibility of a next run first */
    if (*runStart >= rangeEnd) {
        return SBFalse;
    }

    /* Get the first entry and filter its attributes */
    entry = AttributeManagerFindEntry(manager, *runStart, &entryIndex);
    AttributeDictionaryFilter(entry->attributes, filterScope, filterGroup, output);

    *runStart = GetAttributeEntryEndIndex(manager, entryIndex);
    entryIndex += 1;

    if (AttributeDictionaryIsEmpty(output)) {
        /* Iterate while no matching attributes exist */
        while (*runStart < rangeEnd) {
            entry = ListGetRef(&manager->_entries, entryIndex);

            /* Stop when matching attributes appear */
            if (AttributeDictionaryMatchAny(entry->attributes, filterScope, filterGroup)) {
                break;
            }

            *runStart = GetAttributeEntryEndIndex(manager, entryIndex);
            entryIndex += 1;
        }
    } else {
        /* Iterate while the filtered attributes remain the same */
        while (*runStart < rangeEnd) {
            entry = ListGetRef(&manager->_entries, entryIndex);

            /* Stop if filtered attributes change */
            if (!AttributeDictionaryMatchAll(entry->attributes, filterScope, filterGroup, output)) {
                break;
            }

            *runStart = GetAttributeEntryEndIndex(manager, entryIndex);
            entryIndex += 1;
        }
    }

    if (*runStart > rangeEnd) {
        *runStart = rangeEnd;
    }

    return SBTrue;
}
