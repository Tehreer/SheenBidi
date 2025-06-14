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

#include <SheenBidi/SBBase.h>
#include <SheenBidi/SBConfig.h>

#include "AtomicUInt.h"

SB_INTERNAL void AtomicUIntInitialize(AtomicUIntRef aui, SBUInteger value)
{
#ifdef USE_C11_ATOMICS
    atomic_init(aui, (AtomicUInt)value);
#elif defined(USE_ATOMIC_BUILTINS)
    __atomic_store_n(aui, (AtomicUInt)value, __ATOMIC_SEQ_CST);
#else
    *aui = (AtomicUInt)value;
#endif
}

SB_INTERNAL SBUInteger AtomicUIntIncrement(AtomicUIntRef aui)
{
#ifdef USE_C11_ATOMICS
    return (SBUInteger)(atomic_fetch_add(aui, 1) + 1);
#elif defined(USE_ATOMIC_BUILTINS)
    return (SBUInteger)__atomic_add_fetch(aui, 1, __ATOMIC_SEQ_CST);
#elif defined(USE_SYNC_BUILTINS)
    return (SBUInteger)__sync_add_and_fetch(aui, 1);
#elif defined(USE_WIN64_INTRINSICS)
    return (SBUInteger)InterlockedIncrement64(aui);
#elif defined(USE_WIN32_INTRINSICS)
    return (SBUInteger)InterlockedIncrement(aui);
#else
    return ++(*aui);
#endif
}

SB_INTERNAL SBUInteger AtomicUIntDecrement(AtomicUIntRef aui)
{
#ifdef USE_C11_ATOMICS
    return (SBUInteger)(atomic_fetch_sub(aui, 1) - 1);
#elif defined(USE_ATOMIC_BUILTINS)
    return (SBUInteger)__atomic_sub_fetch(aui, 1, __ATOMIC_SEQ_CST);
#elif defined(USE_SYNC_BUILTINS)
    return (SBUInteger)__sync_sub_and_fetch(aui, 1);
#elif defined(USE_WIN64_INTRINSICS)
    return (SBUInteger)InterlockedDecrement64(aui);
#elif defined(USE_WIN32_INTRINSICS)
    return (SBUInteger)InterlockedDecrement(aui);
#else
    return --(*aui);
#endif
}
