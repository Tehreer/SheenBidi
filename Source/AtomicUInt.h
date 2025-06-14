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

#ifndef _SB_INTERNAL_ATOMIC_UINT_H
#define _SB_INTERNAL_ATOMIC_UINT_H

#include <SheenBidi/SBBase.h>
#include <SheenBidi/SBConfig.h>

/* ---------- Compiler Version Detection ---------- */

/* Detect GCC and its version */
#if defined(__GNUC__) && !defined(__clang__)
#define GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#else
#define GCC_VERSION 0
#endif

/* Detect Clang and its version */
#if defined(__clang__)
#define CLANG_VERSION (__clang_major__ * 100 + __clang_minor__)
#else
#define CLANG_VERSION 0
#endif


/* ---------- Atomic Support Feature Detection ---------- */

/* Prefer C11 atomics if available */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)
#define USE_C11_ATOMICS

/* Fallback to compiler intrinsics for GCC/Clang */
#elif defined(__GNUC__) || defined(__clang__)
/* Modern __atomic builtins (GCC 4.7+, Clang 3.1+) */
#if GCC_VERSION >= 407 || CLANG_VERSION >= 301
#define USE_ATOMIC_BUILTINS
/* Legacy __sync builtins (GCC 4.1+, Clang 1.0+) */
#elif GCC_VERSION >= 401 || CLANG_VERSION >= 100
#define USE_SYNC_BUILTINS
#endif

/* Windows-specific intrinsics */
#elif defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__)
#if defined(_WIN64)
#define USE_WIN64_INTRINSICS
#elif defined(_WIN32)
#define USE_WIN32_INTRINSICS
#endif

#endif


/* ---------- AtomicUInt Type Definition ---------- */

#ifdef USE_C11_ATOMICS
#include <stdatomic.h>
typedef atomic_size_t AtomicUInt;
#elif defined(USE_ATOMIC_BUILTINS) || defined(USE_SYNC_BUILTINS)
#include <stddef.h>
typedef size_t AtomicUInt;
#elif defined(USE_WIN64_INTRINSICS)
#include <windows.h>
typedef volatile LONG64 AtomicUInt;
#elif defined(USE_WIN32_INTRINSICS)
#include <windows.h>
typedef volatile LONG AtomicUInt;
#elif defined(SB_CONFIG_ALLOW_NON_ATOMIC_FALLBACK)
typedef SBUInteger AtomicUInt;
#else
#error "No atomic operations available. For thread-unsafe reference counting, manually define \
`SB_CONFIG_ALLOW_NON_ATOMIC_FALLBACK`."
#endif

typedef AtomicUInt *AtomicUIntRef;


/* ---------- API ---------- */

SB_INTERNAL void AtomicUIntInitialize(AtomicUIntRef aui, SBUInteger value);

SB_INTERNAL SBUInteger AtomicUIntIncrement(AtomicUIntRef aui);
SB_INTERNAL SBUInteger AtomicUIntDecrement(AtomicUIntRef aui);

#endif
