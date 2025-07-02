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

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

#include <SheenBidi/SBBase.h>
#include <SheenBidi/SBConfig.h>

extern "C" {
#include <Source/SBAllocator.h>
}

#include "AllocatorTests.h"

using namespace std;
using namespace SheenBidi;

void AllocatorTests::run() {
#ifdef SB_CONFIG_UNITY
    cout << "Most of the allocator test cases will be skipped in unity mode." << endl;
#elif defined(SB_CONFIG_DISABLE_SCRATCH_MEMORY)
    cout << "Scratch memory is disabled due to which related test cases will be skipped." << endl;
#endif

    testBasicBlockAllocation();
    testLargeAllocation();
    testBasicReallocation();
    testReallocationFromNull();
    testBasicScratchAllocation();
    testScratchMemoryReuse();
    testScratchMemoryAlignment();
    testThreadSafeBlockAllocation();
    testThreadLocalScratchMemory();
    testMaximumSizeAllocation();
    testScratchMemoryExhaustion();
    testCustomAllocatorProtocol();
}

void AllocatorTests::testBasicBlockAllocation() {
#if !defined(SB_CONFIG_UNITY)
    SBAllocatorRef allocator = SBAllocatorGetCurrent();

    void *pointer = SBAllocatorAllocateBlock(allocator, 100);
    assert(pointer != nullptr);
    
    SBAllocatorDeallocateBlock(allocator, pointer);
#endif
}

void AllocatorTests::testLargeAllocation() {
#if !defined(SB_CONFIG_UNITY)
    SBAllocatorRef allocator = SBAllocatorGetCurrent();

    const size_t blockSize = 1 << 25;  // 32MB
    void *pointer = SBAllocatorAllocateBlock(allocator, blockSize);

    assert(pointer != nullptr);
    memset(pointer, 0xAA, blockSize);  // Test we can actually use the memory

    SBAllocatorDeallocateBlock(allocator, pointer);
#endif
}

void AllocatorTests::testBasicReallocation() {
#if !defined(SB_CONFIG_UNITY)
    SBAllocatorRef allocator = SBAllocatorGetCurrent();

    const size_t initialSize = 100;
    const size_t newSize = 200;

    void *pointer = SBAllocatorAllocateBlock(allocator, initialSize);
    assert(pointer != nullptr);

    // Fill with pattern
    memset(pointer, 0xBB, initialSize);

    void *newPointer = SBAllocatorReallocateBlock(allocator, pointer, newSize);
    assert(pointer != nullptr);

    // Verify content was preserved
    for (size_t i = 0; i < initialSize; i++) {
        assert(((uint8_t *)newPointer)[i] == 0xBB);
    }

    SBAllocatorDeallocateBlock(allocator, newPointer);
#endif
}

void AllocatorTests::testReallocationFromNull() {
#if !defined(SB_CONFIG_UNITY)
    SBAllocatorRef allocator = SBAllocatorGetCurrent();

    void *pointer = SBAllocatorReallocateBlock(allocator, nullptr, 100);
    assert(pointer != nullptr);  // Should behave like malloc

    SBAllocatorDeallocateBlock(allocator, pointer);
#endif
}

void AllocatorTests::testBasicScratchAllocation() {
#if !defined(SB_CONFIG_UNITY) && !defined(SB_CONFIG_DISABLE_SCRATCH_MEMORY)
    SBAllocatorRef allocator = SBAllocatorGetCurrent();

    void *pointer = SBAllocatorAllocateScratch(allocator, 100);
    assert(pointer != nullptr);

    // Should be able to use the memory
    memset(pointer, 0xCC, 100);

    SBAllocatorResetScratch(allocator);
#endif
}

void AllocatorTests::testScratchMemoryReuse() {
#if !defined(SB_CONFIG_UNITY) && !defined(SB_CONFIG_DISABLE_SCRATCH_MEMORY)
    SBAllocatorRef allocator = SBAllocatorGetCurrent();

    void *pointer1 = SBAllocatorAllocateScratch(allocator, 100);
    assert(pointer1 != nullptr);

    SBAllocatorResetScratch(allocator);

    void *pointer2 = SBAllocatorAllocateScratch(allocator, 100);
    assert(pointer2 != nullptr);
    
    // Should reuse the same memory after reset
    assert(pointer1 == pointer2);

    SBAllocatorResetScratch(allocator);
#endif
}

void AllocatorTests::testScratchMemoryAlignment() {
#if !defined(SB_CONFIG_UNITY) && !defined(SB_CONFIG_DISABLE_SCRATCH_MEMORY)
    SBAllocatorRef allocator = SBAllocatorGetCurrent();

    const size_t sizes[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 29, 31, 37, 61 };
    const size_t align = sizeof(uintptr_t);

    vector<void *> allocations;

    for (auto s : sizes) {
        void *pointer = SBAllocatorAllocateScratch(allocator, s);
        if (!pointer) {
            break;  // No more scratch memory available
        }
        allocations.push_back(pointer);

        // Verify the pointer is aligned
        assert((reinterpret_cast<uintptr_t>(pointer) % align) == 0);
    }

    // Verify all allocations are unique
    for (size_t i = 0; i < allocations.size(); i++) {
        for (size_t j = i + 1; j < allocations.size(); ++j) {
            assert(allocations[i] != allocations[j]);
        }
    }

    SBAllocatorResetScratch(allocator);
#endif
}

void AllocatorTests::testThreadSafeBlockAllocation() {
#if !defined(SB_CONFIG_UNITY)
    SBAllocatorRef allocator = SBAllocatorGetCurrent();

    constexpr int NumThreads = 8;
    constexpr int AllocationsPerThread = 1000;

    vector<thread> threads;
    vector<void *> allocations(NumThreads * AllocationsPerThread, nullptr);

    auto worker = [&](size_t id) {
        for (size_t i = 0; i < AllocationsPerThread; i++) {
            size_t index = (id * AllocationsPerThread) + i;
            allocations[index] = SBAllocatorAllocateBlock(allocator, 100);
            assert(allocations[index] != nullptr);
            memset(allocations[index], id + i, 100);  // Use the memory
        }
    };

    for (size_t i = 0; i < NumThreads; i++) {
        threads.emplace_back(worker, i);
    }

    for (auto &t : threads) {
        t.join();
    }

    // Verify all allocations are unique and valid
    for (size_t i = 0; i < allocations.size(); i++) {
        for (size_t j = i + 1; j < allocations.size(); ++j) {
            assert(allocations[i] != allocations[j]);
        }

        SBAllocatorDeallocateBlock(allocator, allocations[i]);
    }
#endif
}

void AllocatorTests::testThreadLocalScratchMemory() {
#if !defined(SB_CONFIG_UNITY) && !defined(SB_CONFIG_DISABLE_SCRATCH_MEMORY)
    SBAllocatorRef allocator = SBAllocatorGetCurrent();

    constexpr int NumThreads = 16;

    vector<thread> threads;
    vector<void *> threadPointers(NumThreads, nullptr);
    atomic<size_t> threadAllocations{0};
    size_t totalAllocations = 0;

    auto worker = [&](size_t id) {
        void *pointer = SBAllocatorAllocateScratch(allocator, 100);
        threadPointers[id] = pointer;
        threadAllocations += 1;

        if (pointer) {
            // Verify this thread has exclusive access to this memory
            memset(pointer, id, 100);
            for (size_t j = 0; j < 100; j++) {
                assert(((uint8_t *)pointer)[j] == (uint8_t)id);
            }
        }

        // Wait until all threads have allocated their scratch memory
        while (threadAllocations != NumThreads) {
           this_thread::sleep_for(chrono::milliseconds(10));
        }

        SBAllocatorResetScratch(allocator);
    };

    for (size_t i = 0; i < NumThreads; i++) {
        threads.emplace_back(worker, i);
    }

    for (auto &t : threads) {
        t.join();
    }

    // Verify all threads got different scratch memory areas
    for (size_t i = 0; i < threadPointers.size(); i++) {
        if (!threadPointers[i]) {
            continue;  // Skip if this thread couldn't allocate scratch memory
        }
        totalAllocations += 1;

        for (size_t j = i + 1; j < threadPointers.size(); j++) {
            assert(threadPointers[i] != threadPointers[j]);
        }
    }

    // Ensure total allocations do not exceed the configured scratch pool size
    assert(totalAllocations == min(SB_CONFIG_SCRATCH_POOL_SIZE, NumThreads));
#endif
}

void AllocatorTests::testMaximumSizeAllocation() {
#if !defined(SB_CONFIG_UNITY)
    SBAllocatorRef allocator = SBAllocatorGetCurrent();

    const size_t maxSize = SIZE_MAX;  // Largest possible size
    void *pointer = SBAllocatorAllocateBlock(allocator, maxSize);
    assert(pointer == nullptr);  // Should fail gracefully
#endif
}

void AllocatorTests::testScratchMemoryExhaustion() {
#if !defined(SB_CONFIG_UNITY) && !defined(SB_CONFIG_DISABLE_SCRATCH_MEMORY)
    SBAllocatorRef allocator = SBAllocatorGetCurrent();

    vector<void *> allocations;
    while (void *pointer = SBAllocatorAllocateScratch(allocator, SB_CONFIG_SCRATCH_BUFFER_SIZE / 4)) {
        allocations.push_back(pointer);
    }

    // Should eventually return null when exhausted
    assert(SBAllocatorAllocateScratch(allocator, 1) == nullptr);

    // Reset should allow new allocations
    SBAllocatorResetScratch(allocator);
    void *pointer = SBAllocatorAllocateScratch(allocator, SB_CONFIG_SCRATCH_BUFFER_SIZE / 2);
    assert(pointer != nullptr);

    SBAllocatorResetScratch(allocator);
#endif
}

void AllocatorTests::testCustomAllocatorProtocol() {
    struct Data {
        uint8_t scratchBuffer[256];
        size_t allocateCount = 0;
        size_t reallocateCount = 0;
        size_t deallocateCount = 0;
        size_t allocateScratchCount = 0;
        size_t resetScratchCount = 0;
        size_t finalizeCount = 0;
    } data;

    auto allocateFunc = [](SBUInteger size, void *info) -> void * {
        auto data = static_cast<Data *>(info);
        data->allocateCount += 1;
        return malloc(size);
    };

    auto reallocateFunc = [](void *pointer, SBUInteger newSize, void *info) -> void * {
        auto data = static_cast<Data *>(info);
        data->reallocateCount += 1;
        return realloc(pointer, newSize);
    };

    auto deallocateFunc = [](void *pointer, void *info) {
        auto data = static_cast<Data *>(info);
        data->deallocateCount += 1;
        free(pointer);
    };

    auto allocateScratchFunc = [](SBUInteger size, void *info) -> void * {
        auto data = static_cast<Data *>(info);
        data->allocateScratchCount += 1;
        return data->scratchBuffer;
    };

    auto resetScratchFunc = [](void *info) {
        auto data = static_cast<Data *>(info);
        data->resetScratchCount += 1;
    };

    auto finalizeFunc = [](void *info) {
        auto data = static_cast<Data *>(info);
        data->finalizeCount += 1;
    };

    SBAllocatorProtocol protocol = {
        allocateFunc, reallocateFunc, deallocateFunc,
        allocateScratchFunc, resetScratchFunc,
        finalizeFunc
    };

    auto customAllocator = SBAllocatorCreate(&protocol, &data);

    void *pointer = SBAllocatorAllocateBlock(customAllocator, 100);
    assert(pointer != nullptr);
    assert(data.allocateCount == 1);

    void *newPointer = SBAllocatorReallocateBlock(customAllocator, pointer, 200);
    assert(newPointer != nullptr);
    assert(data.reallocateCount == 1);

    SBAllocatorDeallocateBlock(customAllocator, newPointer);
    assert(data.deallocateCount == 1);

    void *scratchPointer = SBAllocatorAllocateScratch(customAllocator, 100);
    assert(scratchPointer == data.scratchBuffer);
    assert(data.allocateScratchCount == 1);

    SBAllocatorResetScratch(customAllocator);
    assert(data.resetScratchCount == 1);

    SBAllocatorRetain(customAllocator);
    assert(data.finalizeCount == 0);

    SBAllocatorRelease(customAllocator);
    assert(data.finalizeCount == 0);

    SBAllocatorRelease(customAllocator);
    assert(data.finalizeCount == 1);
}

#ifdef STANDALONE_TESTING

int main(int argc, const char *argv[]) {
    AllocatorTests allocatorTests;
    allocatorTests.run();

    return 0;
}

#endif
