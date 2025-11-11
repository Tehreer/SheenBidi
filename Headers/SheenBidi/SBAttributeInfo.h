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

#ifndef _SB_PUBLIC_ATTRIBUTE_INFO_H
#define _SB_PUBLIC_ATTRIBUTE_INFO_H

#include <SheenBidi/SBBase.h>

SB_EXTERN_C_BEGIN

enum {
    SBAttributeIDNone = 0           /**< No Attribute */
};
/**
 * Opaque numeric identifier assigned by the registry to an attribute.
 * 
 * Attribute IDs are used everywhere in the API to refer to known attributes; clients can fetch
 * metadata via the attribute registry.
 */
typedef SBUInteger SBAttributeID;

enum {
    SBAttributeGroupNone = 0        /**< No Special Grouping */
};
/**
 * Abstract grouping for attributes (e.g., shaping, paint, language).
 * 
 * Groups can be used for filtering and coalescing strategies.
 */
typedef SBUInt8 SBAttributeGroup;

enum {
    SBAttributeScopeCharacter = 0,  /**< Applies only to the exact character range. */
    SBAttributeScopeParagraph = 1   /**< Applies to entire paragraphs intersecting range. */
};
/**
 * Application scope for an attribute.
 */
typedef SBUInt8 SBAttributeScope;

/**
 * Function to compare two attribute values for equality.
 * 
 * @param first
 *      First attribute value pointer (as previously provided by client).
 * @param second
 *      Second attribute value pointer.
 * @return
 *      `SBTrue` if equal, `SBFalse` otherwise.
 */
typedef SBBoolean (*SBAttributeEqualFunc)(const void *first, const void *second);

/**
 * Function to retain an attribute value at assignment time.
 * 
 * @param value
 *      Attribute value provided by client.
 * @return
 *      Retained value to store internally.
 */
typedef const void *(*SBAttributeRetainFunc)(const void *value);

/**
 * Function to release an attribute value upon removal.
 * 
 * @param value
 *      Attribute value previously retained.
 */
typedef void (*SBAttributeReleaseFunc)(const void *value);

/**
 * Lifecycle and equality callbacks for a particular attribute ID.
 */
typedef struct _SBAttributeCallbacks {
    /**
     * Equality function (optional). If `NULL`, pointer-equality is used.
     */
    SBAttributeEqualFunc equal;
    /**
     * Retain function (optional). If `NULL`, value stored as-is.
     */
    SBAttributeRetainFunc retain;
    /**
     * Release function (optional). If `NULL`, nothing is done on removal.
     */
    SBAttributeReleaseFunc release;
} SBAttributeCallbacks;

/**
 * Public description of a registered attribute.
 */
typedef struct _SBAttributeInfo {
    /**
     * `NULL` terminated unique attribute name.
     */
    const char *name;
    /**
     * Abstract group used for filtering and run coalescing policies.
     */
    SBAttributeGroup group;
    /**
     * Application scope (character/paragraph/line).
     */
    SBAttributeScope scope;
    /**
     * Lifecycle and equality handling for attribute values.
     */
    SBAttributeCallbacks callbacks;
} SBAttributeInfo;

SB_EXTERN_C_END

#endif
