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

#ifndef _SHEENBIDI_TEXT_CONFIG_H
#define _SHEENBIDI_TEXT_CONFIG_H

#include <SheenBidi/SBAttributeRegistry.h>
#include <SheenBidi/SBBase.h>
#include <SheenBidi/SBTextType.h>

#if SB_TEXT_API_SUPPORTED

SB_EXTERN_C_BEGIN

/**
 * Opaque reference to a configuration object required for text creation.
 */
typedef struct _SBTextConfig *SBTextConfigRef;

/**
 * Callback function to retain a paragraph's userInfo value.
 *
 * @param userInfo
 *      Pointer to the userInfo value to retain.
 * @return
 *      Pointer to the retained userInfo value.
 */
typedef const void *(*SBParagraphUserInfoRetainCallback)(const void *userInfo);

/**
 * Callback function to release a paragraph's userInfo value.
 *
 * @param userInfo
 *      Pointer to the userInfo value to release.
 */
typedef void (*SBParagraphUserInfoReleaseCallback)(const void *userInfo);

/**
 * Lifecycle callbacks for paragraph userInfo management.
 */
typedef struct _SBParagraphUserInfoCallbacks {
    /**
     * Value retention function (optional). If `NULL`, values are stored as-is without
     * modification.
     */
    SBParagraphUserInfoRetainCallback retain;
    /**
     * Value release function (optional). If `NULL`, no cleanup is performed when a paragraph's
     * userInfo is cleared.
     */
    SBParagraphUserInfoReleaseCallback release;
} SBParagraphUserInfoCallbacks;

/**
 * Callback invoked immediately whenever a paragraph's userInfo becomes `NULL` (freshly
 * (re)segmented by an edit, or invalidated by an attribute change), so the consumer can (re)shape
 * it and hand back a replacement value without needing to separately scan for invalidated
 * paragraphs.
 *
 * @param text
 *      The text the paragraph belongs to (borrowed; read-only for the duration of the callback).
 *      Only the passed `[index, length)` paragraph is guaranteed fully analyzed at this point;
 *      others may not be if this fires while flushing a batch of edits.
 * @param index
 *      Start index (in code units) of the paragraph needing userInfo.
 * @param length
 *      Length (in code units) of the paragraph.
 * @param context
 *      The context pointer supplied to `SBTextConfigSetParagraphUserInfoProvider`.
 * @return
 *      The value to store (retained via the registered retain callback, if any).
 *
 * @warning
 *      Must not mutate the `text` it receives.
 */
typedef const void *(*SBParagraphUserInfoProviderCallback)(SBTextRef text, SBUInteger index,
    SBUInteger length, void *context);

/**
 * Creates an empty text config instance.
 *
 * @return
 *      A reference to the text config instance, or `NULL` on failure.
 * @note
 *      Must be populated with an attribute registry before text creation. The base level
 *      defaults to `SBLevelDefaultLTR` until changed via `SBTextConfigSetBaseLevel`.
 */
SB_PUBLIC SBTextConfigRef SBTextConfigCreate(void);

/**
 * Sets the attribute registry used by any text created with this config.
 *
 * @param config
 *      The text config object.
 * @param attributeRegistry
 *      Attribute registry reference; the text config retains it, releasing any registry
 *      previously set.
 */
SB_PUBLIC void SBTextConfigSetAttributeRegistry(SBTextConfigRef config,
    SBAttributeRegistryRef attributeRegistry);

/**
 * Sets the base paragraph level policy for newly created texts.
 *
 * @param config
 *      The text config object.
 * @param baseLevel
 *      `SBLevelDefaultLTR`, `SBLevelDefaultRTL`, or a concrete level.
 */
SB_PUBLIC void SBTextConfigSetBaseLevel(SBTextConfigRef config, SBLevel baseLevel);

/**
 * Sets the lifecycle callbacks used to manage each paragraph's userInfo pointer for any text
 * created with this config.
 *
 * @param config
 *      The text config object.
 * @param callbacks
 *      Pointer to an `SBParagraphUserInfoCallbacks` structure defining the retain/release
 *      callbacks; copied by value. Can be `NULL`, in which case userInfo values are stored as-is
 *      with no lifecycle management.
 */
SB_PUBLIC void SBTextConfigSetParagraphUserInfoCallbacks(SBTextConfigRef config,
    const SBParagraphUserInfoCallbacks *callbacks);

/**
 * Sets the callback used to (re)populate paragraph userInfo automatically whenever it's
 * invalidated, for any text created with this config. Fixed at creation so that no paragraph can
 * ever go un-provided by having a provider registered only after some edits have already
 * happened. There is no way to change the provider on an existing text.
 *
 * @param config
 *      The text config object.
 * @param provider
 *      Callback to invoke, or `NULL` to leave every paragraph's userInfo unset.
 * @param context
 *      Opaque pointer passed back to the callback unchanged.
 */
SB_PUBLIC void SBTextConfigSetParagraphUserInfoProvider(SBTextConfigRef config,
    SBParagraphUserInfoProviderCallback provider, void *context);

/**
 * Increments the reference count of a text config object.
 *
 * @param config
 *      The text config object whose reference count will be incremented.
 * @return
 *      The same text config object passed in as the parameter.
 */
SB_PUBLIC SBTextConfigRef SBTextConfigRetain(SBTextConfigRef config);

/**
 * Decrements the reference count of a text config object. The object will be deallocated when
 * its reference count reaches zero.
 *
 * @param config
 *      The text config object whose reference count will be decremented.
 */
SB_PUBLIC void SBTextConfigRelease(SBTextConfigRef config);

SB_EXTERN_C_END

#endif

#endif
