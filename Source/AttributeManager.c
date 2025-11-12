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
 *
 * @param cache
 *      The cache structure to initialize.
 */
static void InitializeAttributeDictionaryCache(AttributeDictionaryCacheRef cache)
{
    /* Initialize the list to hold cached attribute dictionaries */
    ListInitialize(&cache->_attributeDicts, sizeof(AttributeDictionaryRef));
}

/**
 * Finalizes an attribute dictionary cache and releases all cached dictionaries.
 *
 * @param cache
 *      The cache structure to finalize.
 */
static void FinalizeAttributeDictionaryCache(AttributeDictionaryCacheRef cache)
{
    SBUInteger entryCount = cache->_attributeDicts.count;
    SBUInteger entryIndex;

    /* Release all cached attribute dictionaries */
    for (entryIndex = 0; entryIndex < entryCount; entryIndex++) {
        AttributeDictionaryRef cachedDictionary = ListGetVal(&cache->_attributeDicts, entryIndex);

        AttributeDictionaryRelease(cachedDictionary);
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
 *      The attribute registry for creating new dictionaries.
 * @return
 *      An attribute dictionary (cleared and ready for use).
 *
 * @note
 *      The returned dictionary has a reference count of 1.
 */
static AttributeDictionaryRef AcquireAttributeDictionaryFromCache(AttributeDictionaryCacheRef cache,
    SBAttributeRegistryRef registry)
{
    AttributeDictionaryRef dictionary = NULL;
    SBUInteger entryCount = cache->_attributeDicts.count;

    if (entryCount == 0) {
        /* Create a new dictionary if the cache is empty */
        dictionary = AttributeDictionaryCreate(registry);
    } else {
        /* Reuse the last cached dictionary */
        dictionary = ListGetVal(&cache->_attributeDicts, entryCount - 1);
        ListRemoveAt(&cache->_attributeDicts, entryCount - 1);
    }

    return dictionary;
}

/**
 * Caches an attribute dictionary for later reuse if it has exactly one
 * reference.
 *
 * The dictionary is only cached if its retain count is 1, indicating it's not
 * in use elsewhere. This prevents caching dictionaries that are still
 * referenced.
 *
 * @param cache
 *      The cache to store the dictionary in.
 * @param dictionary
 *      The attribute dictionary to cache.
 *
 * @note
 *      The dictionary is cleared before caching. If the retain count is not 1,
 *      the dictionary is not cached (it's still in use elsewhere).
 */
static void StoreAttributeDictionaryInCache(AttributeDictionaryCacheRef cache,
    AttributeDictionaryRef dictionary)
{
    SBUInteger retainCount = ObjectGetRetainCount(dictionary);

    /* Only cache if this is the last reference */
    if (retainCount == 1) {
        /* Clear the dictionary for reuse */
        AttributeDictionaryClear(dictionary);
        /* Retain for the cache */
        AttributeDictionaryRetain(dictionary);
        /* Add to the cache */
        ListAdd(&cache->_attributeDicts, &dictionary);
    }
}

/* =========================================================================
 * Attribute Manager Implementation
 * ========================================================================= */

/**
 * Expands the range start to include the beginning of the first paragraph.
 *
 * @param manager
 *      The attribute manager containing the text.
 * @param[in,out] rangeStart
 *      Pointer to the range start index to modify.
 */
static void ExpandRangeToIncludeFirstParagraph(AttributeManagerRef manager,
    SBUInteger *rangeStart)
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
static void ExpandRangeToIncludeLastParagraph(AttributeManagerRef manager,
    SBUInteger *rangeEnd)
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
 * Expands a range to fully include boundary paragraphs if they're partially
 * covered.
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
 * Shrinks the range start to exclude the first paragraph if it's not fully
 * covered.
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
 * Shrinks the range end to exclude the last paragraph if it's not fully
 * covered.
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
 * Applies a set of attribute items over a specified range of code units.
 *
 * For each code unit in the range, this function ensures an attribute dictionary
 * exists and adds/updates all specified attribute items.
 *
 * @param manager
 *      The attribute manager to modify.
 * @param registry
 *      The attribute registry for managing attributes.
 * @param index
 *      The starting code unit index.
 * @param length
 *      The number of code units in the range.
 * @param attributeItems
 *      Array of attribute items to apply.
 * @param itemCount
 *      Number of items in the attributeItems array.
 * @return
 *      SBTrue on success, SBFalse on failure.
 */
static SBBoolean ApplyAttributeItemsOverRange(AttributeManagerRef manager,
    SBAttributeRegistryRef registry, SBUInteger index, SBUInteger length,
    const SBAttributeItem *attributeItems, SBUInteger itemCount)
{
    SBBoolean succeeded = SBTrue;
    SBUInteger rangeStart = index;
    SBUInteger rangeEnd = rangeStart + length;
    SBUInteger codeUnitIndex;

    /* Iterate through each code unit in the range */
    for (codeUnitIndex = rangeStart; codeUnitIndex < rangeEnd; codeUnitIndex++) {
        AttributeDictionaryRef dictionary = ListGetVal(&manager->attributeDicts, codeUnitIndex);

        /* Acquire or retrieve an attribute dictionary if it doesn't exist */
        if (!dictionary) {
            dictionary = AcquireAttributeDictionaryFromCache(&manager->_cache, registry);
            ListSetVal(&manager->attributeDicts, codeUnitIndex, dictionary);

            succeeded = (dictionary != NULL);
        }

        if (succeeded) {
            SBUInteger itemIndex;

            /* Add all attribute items to the dictionary */
            for (itemIndex = 0; itemIndex < itemCount; itemIndex++) {
                const SBAttributeItem *currentItem = &attributeItems[itemIndex];

                succeeded = AttributeItemListAdd(&dictionary->_list, registry, currentItem);

                if (!succeeded) {
                    goto Exit;
                }
            }
        }
    }

Exit:
    return succeeded;
}

/**
 * Removes a specific attribute from all code units in a range.
 *
 * @param manager
 *      The attribute manager to modify.
 * @param index
 *      The starting code unit index.
 * @param length
 *      The number of code units in the range.
 * @param attributeID
 *      The ID of the attribute to remove.
 */
static void RemoveAttributeFromRange(AttributeManagerRef manager,
    SBUInteger index, SBUInteger length, SBAttributeID attributeID)
{
    SBUInteger rangeEnd = index + length;
    SBUInteger codeUnitIndex;

    /* Remove the attribute from all dictionaries in the specified range */
    for (codeUnitIndex = index; codeUnitIndex < rangeEnd; codeUnitIndex++) {
        AttributeDictionaryRef dictionary = ListGetVal(&manager->attributeDicts, codeUnitIndex);

        if (dictionary) {
            /* Remove the attribute ID */
            AttributeDictionaryRemove(dictionary, manager->_registry, attributeID);

            /* Cache and clear the dictionary if it becomes empty */
            if (AttributeDictionaryIsEmpty(dictionary)) {
                StoreAttributeDictionaryInCache(&manager->_cache, dictionary);
                AttributeDictionaryRelease(dictionary);

                /* Set to NULL to indicate no attributes at this position */
                ListSetVal(&manager->attributeDicts, codeUnitIndex, NULL);
            }
        }
    }
}

/**
 * Adjusts paragraph-scoped attributes after text has been removed.
 *
 * When text is removed, two previously separate paragraphs may merge. This
 * function ensures that paragraph-scoped attributes are properly propagated
 * across the merged paragraph boundary.
 *
 * @param manager
 *      The attribute manager to adjust.
 * @param mergePointIndex
 *      The code unit index where the merge occurred.
 * @return
 *      SBTrue on success, SBFalse on failure.
 */
static SBBoolean AdjustParagraphAttributesAfterMerge(AttributeManagerRef manager,
    SBUInteger mergePointIndex)
{
    SBBoolean succeeded = SBTrue;

    if (mergePointIndex > 0 && mergePointIndex < manager->attributeDicts.count) {
        SBAttributeRegistryRef registry = manager->_registry;
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
            AttributeItemListRef paragraphAttributes;
            AttributeDictionaryRef sourceDictionary;

            paragraphAttributes = &manager->_tempList;

            /* Extract paragraph-scoped attributes from before the merge point */
            sourceDictionary = ListGetVal(&manager->attributeDicts, precedingIndex);
            succeeded = AttributeDictionaryFilter(sourceDictionary, SBAttributeScopeParagraph,
                SBAttributeGroupNone, paragraphAttributes);

            if (succeeded && paragraphAttributes->count > 0) {
                /* Apply to the second half of the merged paragraph */
                succeeded = ApplyAttributeItemsOverRange(manager, registry,
                    mergePointIndex, paragraphEnd - mergePointIndex,
                    sourceDictionary->_list.items, sourceDictionary->_list.count);
            }

            if (succeeded) {
                /* Extract paragraph-scoped attributes from after the merge point */
                sourceDictionary = ListGetVal(&manager->attributeDicts, mergePointIndex);
                succeeded = AttributeDictionaryFilter(sourceDictionary, SBAttributeScopeParagraph,
                    SBAttributeGroupNone, paragraphAttributes);
            }

            if (succeeded && paragraphAttributes->count > 0) {
                /* Apply to the first half of the merged paragraph */
                succeeded = ApplyAttributeItemsOverRange(manager, registry,
                    paragraphStart, mergePointIndex - paragraphStart,
                    sourceDictionary->_list.items, sourceDictionary->_list.count);
            }
        }
    }

    return succeeded;
}

SB_INTERNAL void AttributeManagerInitialize(AttributeManagerRef manager,
    SBTextRef parent, SBAttributeRegistryRef registry)
{
    manager->parent = parent;
    manager->_registry = registry;

    if (registry) {
        /* Initialize cache and storage list only if registry is provided */
        InitializeAttributeDictionaryCache(&manager->_cache);
        ListInitialize(&manager->_tempList, sizeof(AttributeItemList));
        ListInitialize(&manager->attributeDicts, sizeof(AttributeDictionaryRef));
    }
}

SB_INTERNAL void AttributeManagerFinalize(AttributeManagerRef manager)
{
    if (manager->_registry) {
        SBUInteger listCount = manager->attributeDicts.count;
        SBUInteger index;

        ListFinalize(&manager->_tempList);

        /* Finalize the cache */
        FinalizeAttributeDictionaryCache(&manager->_cache);

        /* Release all attribute dictionaries */
        for (index = 0; index < listCount; index++) {
            AttributeDictionaryRef dictionary = ListGetVal(&manager->attributeDicts, index);

            if (dictionary) {
                AttributeDictionaryRelease(dictionary);
            }
        }

        ListFinalize(&manager->attributeDicts);
    }
}

SB_INTERNAL SBBoolean AttributeManagerCopyAttributes(AttributeManagerRef manager,
    const AttributeManager *source)
{
    SBBoolean succeeded = SBTrue;

    if (manager->_registry) {
        SBUInteger dictCount = source->attributeDicts.count;
        SBUInteger dictIndex;

        /* Reserve space in the attribute dictionary list */
        succeeded = ListReserveRange(&manager->attributeDicts, 0, dictCount);

        if (succeeded) {
            /* Initialize the reserved range */
            for (dictIndex = 0; dictIndex < dictCount; dictIndex++) {
                AttributeDictionaryRef sourceDict = ListGetVal(&source->attributeDicts, dictIndex);
                AttributeDictionaryRef newDict = NULL;

                if (sourceDict) {
                    newDict = AttributeDictionaryCopy(sourceDict);
                }

                ListSetVal(&manager->attributeDicts, dictIndex, newDict);
            }
        }
    }

    return succeeded;
}

SB_INTERNAL SBBoolean AttributeManagerReserveRange(AttributeManagerRef manager,
    SBUInteger index, SBUInteger length)
{
    SBBoolean succeeded = SBTrue;

    /* Length MUST be greater than 0 */
    SBAssert(length > 0);

    if (manager->_registry) {
        /* Reserve space in the attribute dictionary list */
        succeeded = ListReserveRange(&manager->attributeDicts, index, length);

        if (succeeded) {
            SBUInteger rangeStart = index;
            SBUInteger rangeEnd = index + length;
            AttributeDictionaryRef sourceDictionary = NULL;
            SBUInteger attributesIndex;

            /* Initialize the reserved range */
            while (rangeStart < rangeEnd) {
                ListSetVal(&manager->attributeDicts, rangeStart, NULL);
                rangeStart += 1;
            }

            /* Determine which code unit's attributes to copy */
            attributesIndex = (index == 0 ? length : index - 1);

            if (attributesIndex < manager->attributeDicts.count) {
                sourceDictionary = ListGetVal(&manager->attributeDicts, attributesIndex);
            }

            if (sourceDictionary) {
                /* Apply the attributes of the adjacent code unit over the reserved range */
                succeeded = ApplyAttributeItemsOverRange(manager, manager->_registry, index, length,
                    sourceDictionary->_list.items, sourceDictionary->_list.count);
            }
        }
    }

    return succeeded;
}

SB_INTERNAL SBBoolean AttributeManagerRemoveRange(AttributeManagerRef manager,
    SBUInteger index, SBUInteger length)
{
    SBUInteger succeeded = SBTrue;
    SBAttributeRegistryRef registry = manager->_registry;

    SBAssert(length > 0);

    if (registry) {
        SBUInteger startIndex = index;
        SBUInteger endIndex = index + length;
        SBUInteger attributesIndex;

        /* Release and cache all attribute dictionaries in the range */
        for (attributesIndex = index; attributesIndex < endIndex; attributesIndex++) {
            AttributeDictionaryRef dictionary = ListGetVal(&manager->attributeDicts, attributesIndex);

            if (dictionary) {
                /* Try to cache the dictionary */
                StoreAttributeDictionaryInCache(&manager->_cache, dictionary);
                /* Release our reference */
                AttributeDictionaryRelease(dictionary);
            }
        }

        /* Remove the range from the attribute list */
        ListRemoveRange(&manager->attributeDicts, index, length);

        /* Adjust paragraph attributes if paragraphs merged */
        succeeded = AdjustParagraphAttributesAfterMerge(manager, startIndex);
    }

    return succeeded;
}

SB_INTERNAL SBBoolean AttributeManagerSetAttribute(AttributeManagerRef manager,
    SBUInteger index, SBUInteger length, SBAttributeID attributeID, const void *attributeValue)
{
    SBBoolean succeeded = SBTrue;
    SBAttributeRegistryRef registry = manager->_registry;

    SBAssert(length > 0);

    if (registry) {
        const SBAttributeInfo *attributeInfo;

        attributeInfo = SBAttributeRegistryGetInfoReference(registry, attributeID);

        if (attributeInfo) {
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

            /* Apply the attribute over the specified range */
            succeeded = ApplyAttributeItemsOverRange(manager, registry, index, length,
                &attributeItem, 1);
        } else {
            succeeded = SBFalse;
        }
    }

    return succeeded;
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
    SBAttributeID attributeID, AttributeItemListRef outputItems)
{
    SBBoolean runFound = SBTrue;
    AttributeDictionaryRef dictionary;
    const SBAttributeItem *initialItem;

    /* Clear the output items list */
    ListRemoveAll(outputItems);

    /* Check for the possibility of a next run first */
    if (*runStart >= rangeEnd) {
        runFound = SBFalse;
        goto Exit;
    }

    /* Get the attribute dictionary of the first code unit and search for the attribute */
    dictionary = ListGetVal(&manager->attributeDicts, *runStart);
    initialItem = AttributeDictionaryFindItem(dictionary, attributeID);

    if (initialItem) {
        /* Add the initial item to the output list */
        runFound = AttributeItemListAdd(outputItems, NULL, initialItem);

        if (runFound) {
            /* Iterate while the attribute value remains the same */
            while (++(*runStart) < rangeEnd) {
                SBBoolean valuesMatched = SBFalse;
                const SBAttributeItem *subsequentItem;

                dictionary = ListGetVal(&manager->attributeDicts, *runStart);
                subsequentItem = AttributeDictionaryFindItem(dictionary, attributeID);

                if (subsequentItem) {
                    /* Check if the attribute value matches */
                    valuesMatched = SBAttributeRegistryIsEqualAttribute(dictionary->_registry,
                        attributeID, initialItem->attributeValue, subsequentItem->attributeValue);
                }

                /* Stop if the value changes */
                if (!valuesMatched) {
                    break;
                }
            }
        }
    } else {
        /* Iterate while the attribute doesn't exist */
        while (++(*runStart) < rangeEnd) {
            const SBAttributeItem *subsequentItem;

            dictionary = ListGetVal(&manager->attributeDicts, *runStart);
            subsequentItem = AttributeDictionaryFindItem(dictionary, attributeID);

            /* Stop when the attribute appears */
            if (subsequentItem) {
                break;
            }
        }
    }

Exit:
    return runFound;
}

SB_INTERNAL SBBoolean AttributeManagerGetOnwardRunByFilteringCollection(AttributeManagerRef manager,
    SBUInteger *runStart, SBUInteger rangeEnd,
    SBAttributeScope filterScope, SBAttributeGroup filterGroup, AttributeItemListRef outputItems)
{
    SBBoolean runFound;
    AttributeDictionaryRef dictionary;

    /* Clear the output items list */
    ListRemoveAll(outputItems);

    /* Check for the possibility of a next run first */
    if (*runStart >= rangeEnd) {
        runFound = SBFalse;
        goto Exit;
    }

    /* Get the attribute dictionary of the first code unit */
    dictionary = ListGetVal(&manager->attributeDicts, *runStart);
    /* Get the items from the dictionary respecting the specified filters */
    runFound = AttributeDictionaryFilter(dictionary, filterScope, filterGroup, outputItems);

    if (runFound) {
        if (outputItems->count > 0) {
            /* Iterate while the filtered attributes remain the same */
            while (++(*runStart) < rangeEnd) {
                dictionary = ListGetVal(&manager->attributeDicts, *runStart);

                if (!AttributeDictionaryMatchAll(dictionary, filterScope, filterGroup, outputItems)) {
                    /* Stop if the attributes change */
                    break;
                }
            }
        } else {
            /* Iterate while no matching attributes exist */
            while (++(*runStart) < rangeEnd) {
                dictionary = ListGetVal(&manager->attributeDicts, *runStart);

                /* Stop when matching attributes appear */
                if (AttributeDictionaryMatchAny(dictionary, filterScope, filterGroup)) {
                    break;
                }
            }
        }
    }

Exit:
    return runFound;
}
