/*
 * Copyright (C) 2026 Muhammad Tayyab Akram
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

#ifndef _SB_INTERNAL_ATTRIBUTE_LIST_H
#define _SB_INTERNAL_ATTRIBUTE_LIST_H

#include <API/SBBase.h>

#if SB_TEXT_API_SUPPORTED

#include <SheenBidi/SBAttributeList.h>
#include <SheenBidi/SBAttributeRegistry.h>

#include <Core/List.h>
#include <Core/Object.h>

typedef struct _SBAttributeList {
    ObjectBase _base;
    SBAttributeRegistryRef registry;
    List _list;
} SBAttributeList;

/**
 * Creates a new, independently-retained attribute list bound to `registry`.
 *
 * The returned list is a genuine `ObjectCreate`-backed object (unlike, for example,
 * `SBUniformRun.attributes`, which is a borrowed view) — callers may retain/release it via
 * `SBAttributeListRetain`/`SBAttributeListRelease`. Its finalizer releases every remaining item's
 * value through `registry` before freeing the underlying storage.
 *
 * @param registry
 *      The attribute registry to bind to the list; retained for the list's lifetime and used to
 *      retain/release attribute values as they are inserted or removed.
 * @return
 *      A new, retained attribute list, or `NULL` on allocation failure.
 */
SB_INTERNAL SBAttributeListRef SBAttributeListCreate(SBAttributeRegistryRef registry);

#define SBAttributeListSize(list_)                          \
    ((list_)->_list.count)

#define SBAttributeListReserveRange(list_, index_, count_)  \
    ListReserveRange(&(list_)->_list, index_, count_)

#define SBAttributeListReserveEnd(list_, count_)            \
    ListReserveRange(&(list_)->_list, (list_)->_list.count, count_)

#define SBAttributeListGetAt(list_, index_)                 \
    ListGetPtr(&(list_)->_list, index_)

#define SBAttributeListGetLast(list_)                       \
    ListGetPtr(&(list_)->_list, (list_)->_list.count - 1)

#define SBAttributeListRemoveAt(list_, index_)              \
    ListRemoveAt(&(list_)->_list, index_)

#define SBAttributeListRemoveAll(list_)                     \
    ListRemoveAll(&(list_)->_list)

/**
 * Finds the index where an attribute with the specified ID is located or should be inserted.
 *
 * @param list
 *      The attribute list to search.
 * @param attributeID
 *      The attribute ID to search for.
 * @param itemFound
 *      Pointer to a boolean that will be set to SBTrue if the attribute is found, SBFalse
 *      otherwise.
 * @return
 *      The index of the found item, or the insertion index if not found.
 */
SB_INTERNAL SBUInteger SBAttributeListBinarySearchIndex(SBAttributeListRef list,
    SBAttributeID attributeID, SBBoolean *itemFound);

#endif

#endif
