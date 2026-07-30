/*
 * Copyright (C) 2014-2026 Muhammad Tayyab Akram
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

#ifndef _SHEENBIDI_CONFIG_H
#define _SHEENBIDI_CONFIG_H

/**
 * Exports public symbols when building SheenBidi as a Windows DLL.
 * Must not be combined with `SB_CONFIG_DLL_IMPORT`.
 */
/* #define SB_CONFIG_DLL_EXPORT */

/**
 * Imports public symbols when consuming SheenBidi as a Windows DLL.
 * Must not be combined with `SB_CONFIG_DLL_EXPORT`.
 */
/* #define SB_CONFIG_DLL_IMPORT */

/**
 * Enables internal debug logging of the bidirectional algorithm to standard output.
 * Intended for diagnosing the library itself; it has no effect on the public API.
 */
/* #define SB_CONFIG_LOG */

/**
 * Builds the library as a single translation unit via `Source/SheenBidi.c`, inlining all
 * internal symbols as `static`. Recommended for consumers that compile SheenBidi directly
 * into their own build.
 */
/* #define SB_CONFIG_UNITY */

/**
 * Permits the library to fall back to non-atomic, thread-unsafe reference counting when no
 * atomic operations are available for the target platform. Without this defined, the library
 * fails to compile in that situation rather than silently becoming thread-unsafe.
 */
/* #define SB_CONFIG_ALLOW_NON_ATOMIC_FALLBACK */

/**
 * Disables the scratch memory functionality of the default allocator.
 * When defined, all scratch memory operations return `NULL` and do nothing.
 */
/* #define SB_CONFIG_DISABLE_SCRATCH_MEMORY */

/**
 * Enables the optional text editing and analysis API, including support for inserting, removing,
 * and modifying code units, applying attributes, and querying logical, script, attribute, and
 * visual runs. These features perform incremental paragraph analysis internally.
 *
 * The API is considered experimental and may change based on production feedback.
 */
/* #define SB_CONFIG_EXPERIMENTAL_TEXT_API */

/**
 * Defines the size of each scratch buffer in bytes.
 * Defaults to 8192 bytes (8KB) if not specified.
 */
#ifndef SB_CONFIG_SCRATCH_BUFFER_SIZE
#define SB_CONFIG_SCRATCH_BUFFER_SIZE 8192
#endif

/**
 * Defines the number of scratch buffers in the pool.
 * Defaults to 3 buffers if not specified.
 */
#ifndef SB_CONFIG_SCRATCH_POOL_SIZE
#define SB_CONFIG_SCRATCH_POOL_SIZE 3
#endif

#endif
