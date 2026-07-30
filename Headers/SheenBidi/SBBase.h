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

#ifndef _SB_PUBLIC_BASE_H
#define _SB_PUBLIC_BASE_H

#include <stdint.h>

/**
 * Wraps a declaration so it is emitted with C linkage when included from C++, and expands to
 * nothing when included from C. Every public header opens with `SB_EXTERN_C_BEGIN` and closes
 * with `SB_EXTERN_C_END`.
 */
#ifdef __cplusplus
#define SB_EXTERN_C_BEGIN extern "C" {
#define SB_EXTERN_C_END }
#else
#define SB_EXTERN_C_BEGIN
#define SB_EXTERN_C_END
#endif

/**
 * Marks a declaration as part of the public API, applying whatever visibility or DLL
 * export/import attribute the target platform and `SB_CONFIG_DLL_EXPORT` /
 * `SB_CONFIG_DLL_IMPORT` require.
 */
#if defined(_WIN32)

#if defined(SB_CONFIG_DLL_EXPORT)
#define SB_PUBLIC __declspec(dllexport)
#elif defined(SB_CONFIG_DLL_IMPORT)
#define SB_PUBLIC __declspec(dllimport)
#endif

#elif defined(__GNUC__)
#define SB_PUBLIC __attribute__((visibility("default")))
#endif

#ifndef SB_PUBLIC
#define SB_PUBLIC
#endif

/**
 * Expands to `1` if the experimental text editing and analysis API was enabled at build time
 * via `SB_CONFIG_EXPERIMENTAL_TEXT_API`, or `0` otherwise.
 */
#if defined(SB_CONFIG_EXPERIMENTAL_TEXT_API)
#define SB_TEXT_API_SUPPORTED 1
#else
#define SB_TEXT_API_SUPPORTED 0
#endif

/**
 * Marks a declaration as deprecated, so that using it triggers a compiler warning where
 * supported. Expands to nothing on compilers without a deprecation attribute.
 */
#if defined(__cplusplus) && __cplusplus >= 201402L
#define SB_DEPRECATED [[deprecated]]
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#define SB_DEPRECATED [[deprecated]]
#elif defined(__clang__) || defined(__GNUC__)
#define SB_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define SB_DEPRECATED __declspec(deprecated)
#else
#define SB_DEPRECATED
#endif

SB_EXTERN_C_BEGIN

/**
 * A type to represent an 8-bit signed integer.
 */
typedef int8_t                      SBInt8;

/**
 * A type to represent a 16-bit signed integer.
 */
typedef int16_t                     SBInt16;

/**
 * A type to represent a 32-bit signed integer.
 */
typedef int32_t                     SBInt32;

/**
 * A type to represent an 8-bit unsigned integer.
 */
typedef uint8_t                     SBUInt8;

/**
 * A type to represent a 16-bit unsigned integer.
 */
typedef uint16_t                    SBUInt16;

/**
 * A type to represent a 32-bit unsigned integer.
 */
typedef uint32_t                    SBUInt32;

/**
 * A signed integer type whose width is equal to the width of the machine word.
 */
typedef intptr_t                    SBInteger;

/**
 * An unsigned integer type whose width is equal to the width of the machine word.
 */
typedef uintptr_t                   SBUInteger;

/**
 * Constants that specify the states of a boolean.
 */
enum {
    SBFalse = 0, /**< A value representing the false state. */
    SBTrue  = 1  /**< A value representing the true state. */
};
/**
 * A type to represent a boolean value.
 */
typedef SBUInt8                     SBBoolean;

/**
 * Returns whether an 8-bit unsigned value lies within an inclusive range.
 *
 * @param v The value to test.
 * @param s The inclusive lower bound of the range.
 * @param e The inclusive upper bound of the range.
 * @return Non-zero if `v` lies within [`s`, `e`], zero otherwise.
 */
#define SBUInt8InRange(v, s, e)     \
(                                   \
    (SBUInt8)((v) - (s))            \
 <= (SBUInt8)((e) - (s))            \
)

/**
 * Returns whether a 16-bit unsigned value lies within an inclusive range.
 *
 * @param v The value to test.
 * @param s The inclusive lower bound of the range.
 * @param e The inclusive upper bound of the range.
 * @return Non-zero if `v` lies within [`s`, `e`], zero otherwise.
 */
#define SBUInt16InRange(v, s, e)    \
(                                   \
    (SBUInt16)((v) - (s))           \
 <= (SBUInt16)((e) - (s))           \
)

/**
 * Returns whether a 32-bit unsigned value lies within an inclusive range.
 *
 * @param v The value to test.
 * @param s The inclusive lower bound of the range.
 * @param e The inclusive upper bound of the range.
 * @return Non-zero if `v` lies within [`s`, `e`], zero otherwise.
 */
#define SBUInt32InRange(v, s, e)    \
(                                   \
    (SBUInt32)((v) - (s))           \
 <= (SBUInt32)((e) - (s))           \
)


/**
 * A type to represent a bidi level.
 */
typedef SBUInt8                     SBLevel;

/**
 * A value representing an invalid bidi level.
 */
#define SBLevelInvalid              0xFF

/**
 * A value representing the maximum explicit embedding level.
 */
#define SBLevelMax                  125

/**
 * A value specifying that the base level should default to zero (left-to-right) when the
 * text contains no strong character.
 */
#define SBLevelDefaultLTR           0xFE

/**
 * A value specifying that the base level should default to one (right-to-left) when the
 * text contains no strong character.
 */
#define SBLevelDefaultRTL           0xFD

SB_EXTERN_C_END

#endif
