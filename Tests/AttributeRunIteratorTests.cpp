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

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

#include <SheenBidi/SBAttributeInfo.h>
#include <SheenBidi/SBAttributeRegistry.h>
#include <SheenBidi/SBText.h>
#include <SheenBidi/SBTextConfig.h>

extern "C" {
#include <Source/SBBase.h>
#include <Source/SBParagraph.h>
#include <Source/SBTextIterators.h>
}

#include "AttributeRunIteratorTests.h"

using namespace std;
using namespace SheenBidi;

using Number = uint32_t;
static const Number Numbers[] = { 0, 1, 2 };

constexpr auto Zero = &Numbers[0];
constexpr auto One = &Numbers[1];
constexpr auto Two = &Numbers[2];

class Typeface {
public:
    static constexpr auto Serif = Zero;
    static constexpr auto SansSerif = One;
    static constexpr auto Monospace = Two;
};

class Alignment {
public:
    static constexpr auto Leading = Zero;
    static constexpr auto Center = One;
    static constexpr auto Trailing = Two;
};

class Attribute {
public:
    static constexpr auto Typeface = "typeface";
    static constexpr auto Alignment = "alignment";
};

static const vector<SBAttributeInfo> TestAttributes = {
    {Attribute::Typeface, 1, SBAttributeScopeCharacter, { nullptr }},
    {Attribute::Alignment, 2, SBAttributeScopeParagraph, { nullptr }}
};

static SBMutableTextRef SBTextCreateTest(const string &str, SBLevel baseLevel = SBLevelDefaultLTR) {
    auto registry = SBAttributeRegistryCreate(TestAttributes.data(), TestAttributes.size());

    auto config = SBTextConfigCreate();
    SBTextConfigSetAttributeRegistry(config, registry);
    SBTextConfigSetBaseLevel(config, baseLevel);

    auto text = SBTextCreateMutable(SBStringEncodingUTF8, config);
    SBTextAppendCodeUnits(text, str.data(), str.length());

    SBTextConfigRelease(config);
    SBAttributeRegistryRelease(registry);

    return text;
}

void AttributeRunIteratorTests::run() {
    testInitialization();
    testBasicIteration();
    testAttributeBoundaries();
    testMultipleAttributes();
    testOverlappingRuns();
    testAttributeMerging();
    testRetainRelease();
    testEdgeCases();
    testComplexScenarios();
}

void AttributeRunIteratorTests::testInitialization() {
    // Test 1: Create iterator with valid text but zero-length
    {
        auto text = SBTextCreateTest("");
        auto iterator = SBAttributeRunIteratorCreate(text);
        assert(iterator != nullptr);

        auto run = SBAttributeRunIteratorGetCurrent(iterator);
        assert(run != nullptr);
        assert(run->index == SBInvalidIndex);
        assert(run->length == 0);
        assert(run->attributeItems == nullptr);
        assert(run->attributeCount == 0);

        assert(!SBAttributeRunIteratorMoveNext(iterator));

        SBAttributeRunIteratorRelease(iterator);
        SBTextRelease(text);
    }

    // Test 2: Create iterator with valid text and non-zero length
    {
        auto text = SBTextCreateTest("ABC");
        auto iterator = SBAttributeRunIteratorCreate(text);
        assert(iterator != nullptr);

        auto run = SBAttributeRunIteratorGetCurrent(iterator);
        assert(run != nullptr);
        assert(run->index == SBInvalidIndex);
        assert(run->length == 0);
        assert(run->attributeItems == nullptr);
        assert(run->attributeCount == 0);

        SBAttributeRunIteratorRelease(iterator);
        SBTextRelease(text);
    }

    // Test 3: Create iterator with invalid attribute type
    {
        auto text = SBTextCreateTest("ABC");
        SBTextSetAttribute(text, 0, 3, 100, Two);

        auto iterator = SBAttributeRunIteratorCreate(text);
        assert(iterator != nullptr);

        auto run = SBAttributeRunIteratorGetCurrent(iterator);
        assert(run != nullptr);
        assert(run->index == SBInvalidIndex);
        assert(run->length == 0);
        assert(run->attributeItems == nullptr);
        assert(run->attributeCount == 0);

        SBAttributeRunIteratorRelease(iterator);
        SBTextRelease(text);
    }
}

void AttributeRunIteratorTests::testBasicIteration() {
    auto text = SBTextCreateTest("Hello");
    auto registry = SBTextGetAttributeRegistry(text);

    // Set a single attribute for the entire text
    auto attributeID = SBAttributeRegistryGetAttributeID(registry, Attribute::Alignment);
    SBTextSetAttribute(text, 2, 1, attributeID, Alignment::Center);

    auto iterator = SBAttributeRunIteratorCreate(text);
    auto run = SBAttributeRunIteratorGetCurrent(iterator);

    SBAttributeRunIteratorSetupAttributeID(iterator, attributeID);

    assert(SBAttributeRunIteratorMoveNext(iterator));
    assert(run->index == 0);
    assert(run->length == 5);
    assert(run->attributeCount == 1);
    assert(run->attributeItems->attributeID == attributeID);
    assert(run->attributeItems->attributeValue == Alignment::Center);

    assert(!SBAttributeRunIteratorMoveNext(iterator));

    SBAttributeRunIteratorRelease(iterator);
    SBTextRelease(text);
}

void AttributeRunIteratorTests::testAttributeBoundaries() {
    auto text = SBTextCreateTest("ABCDEF");
    auto registry = SBTextGetAttributeRegistry(text);

    // Set different attributes for different ranges
    auto attributeID = SBAttributeRegistryGetAttributeID(registry, Attribute::Typeface);
    SBTextSetAttribute(text, 0, 2, attributeID, Typeface::Serif);
    SBTextSetAttribute(text, 2, 2, attributeID, Typeface::SansSerif);
    SBTextSetAttribute(text, 4, 2, attributeID, Typeface::Monospace);

    auto iterator = SBAttributeRunIteratorCreate(text);
    auto run = SBAttributeRunIteratorGetCurrent(iterator);

    // First run
    assert(SBAttributeRunIteratorMoveNext(iterator));
    assert(run->index == 0);
    assert(run->length == 2);
    assert(run->attributeCount == 1);
    assert(run->attributeItems->attributeID == attributeID);
    assert(run->attributeItems->attributeValue == Typeface::Serif);

    // Second run
    assert(SBAttributeRunIteratorMoveNext(iterator));
    assert(run->index == 2);
    assert(run->length == 2);
    assert(run->attributeCount == 1);
    assert(run->attributeItems->attributeID == attributeID);
    assert(run->attributeItems->attributeValue == Typeface::SansSerif);

    // Third run
    assert(SBAttributeRunIteratorMoveNext(iterator));
    assert(run->index == 4);
    assert(run->length == 2);
    assert(run->attributeCount == 1);
    assert(run->attributeItems->attributeID == attributeID);
    assert(run->attributeItems->attributeValue == Typeface::Monospace);

    assert(!SBAttributeRunIteratorMoveNext(iterator));

    SBAttributeRunIteratorRelease(iterator);
    SBTextRelease(text);
}

void AttributeRunIteratorTests::testMultipleAttributes() {
    auto text = SBTextCreateTest("Test");
    auto registry = SBTextGetAttributeRegistry(text);

    // Set attributes of different types
    auto typefaceID = SBAttributeRegistryGetAttributeID(registry, Attribute::Typeface);
    auto alignmentID = SBAttributeRegistryGetAttributeID(registry, Attribute::Alignment);

    SBTextSetAttribute(text, 0, 4, typefaceID, Typeface::SansSerif);
    SBTextSetAttribute(text, 0, 1, alignmentID, Alignment::Leading);

    auto iterator = SBAttributeRunIteratorCreate(text);
    auto run = SBAttributeRunIteratorGetCurrent(iterator);

    SBAttributeRunIteratorSetupAttributeID(iterator, typefaceID);

    assert(SBAttributeRunIteratorMoveNext(iterator));
    assert(run->index == 0);
    assert(run->length == 4);
    assert(run->attributeCount == 1);
    assert(run->attributeItems->attributeID == typefaceID);
    assert(run->attributeItems->attributeValue == Typeface::SansSerif);
    assert(!SBAttributeRunIteratorMoveNext(iterator));

    SBAttributeRunIteratorSetupAttributeID(iterator, alignmentID);

    assert(SBAttributeRunIteratorMoveNext(iterator));
    assert(run->index == 0);
    assert(run->length == 4);
    assert(run->attributeCount == 1);
    assert(run->attributeItems->attributeID == alignmentID);
    assert(run->attributeItems->attributeValue == Alignment::Leading);
    assert(!SBAttributeRunIteratorMoveNext(iterator));

    SBAttributeRunIteratorRelease(iterator);
    SBTextRelease(text);
}

void AttributeRunIteratorTests::testOverlappingRuns() {
    auto text = SBTextCreateTest("Sheen");
    auto registry = SBTextGetAttributeRegistry(text);

    // Create overlapping runs
    auto attributeID = SBAttributeRegistryGetAttributeID(registry, Attribute::Typeface);
    SBTextSetAttribute(text, 0, 3, attributeID, Typeface::SansSerif);
    SBTextSetAttribute(text, 2, 3, attributeID, Typeface::Monospace);

    auto iterator = SBAttributeRunIteratorCreate(text);
    auto run = SBAttributeRunIteratorGetCurrent(iterator);

    // Verify the resulting runs
    assert(SBAttributeRunIteratorMoveNext(iterator));
    assert(run->index == 0);
    assert(run->length == 2);
    assert(run->attributeCount == 1);
    assert(run->attributeItems->attributeID == attributeID);
    assert(run->attributeItems->attributeValue == Typeface::SansSerif);

    assert(SBAttributeRunIteratorMoveNext(iterator));
    assert(run->index == 2);
    assert(run->length == 3);
    assert(run->attributeCount == 1);
    assert(run->attributeItems->attributeID == attributeID);
    assert(run->attributeItems->attributeValue == Typeface::Monospace);

    assert(!SBAttributeRunIteratorMoveNext(iterator));

    SBAttributeRunIteratorRelease(iterator);
    SBTextRelease(text);
}

void AttributeRunIteratorTests::testAttributeMerging() {
    auto text = SBTextCreateTest("World");
    auto registry = SBTextGetAttributeRegistry(text);

    // Set same attribute value in adjacent ranges
    auto attributeID = SBAttributeRegistryGetAttributeID(registry, Attribute::Typeface);
    SBTextSetAttribute(text, 0, 2, attributeID, Typeface::SansSerif);
    SBTextSetAttribute(text, 2, 3, attributeID, Typeface::SansSerif);

    auto iterator = SBAttributeRunIteratorCreate(text);
    auto run = SBAttributeRunIteratorGetCurrent(iterator);

    // Verify runs are merged
    assert(SBAttributeRunIteratorMoveNext(iterator));
    assert(run->index == 0);
    assert(run->length == 5);
    assert(run->attributeCount == 1);
    assert(run->attributeItems->attributeID == attributeID);
    assert(run->attributeItems->attributeValue == Typeface::SansSerif);

    SBAttributeRunIteratorRelease(iterator);
    SBTextRelease(text);
}

void AttributeRunIteratorTests::testRetainRelease() {
    auto text = SBTextCreateTest("ABC");
    auto iterator = SBAttributeRunIteratorCreate(text);

    // Test retain/release
    auto retained = SBAttributeRunIteratorRetain(iterator);
    assert(retained == iterator);

    SBAttributeRunIteratorRelease(retained);
    SBAttributeRunIteratorRelease(iterator);
    SBTextRelease(text);
}

void AttributeRunIteratorTests::testEdgeCases() {
    // Test 1: Single character text with attribute
    {
        auto text = SBTextCreateTest("A");
        auto registry = SBTextGetAttributeRegistry(text);

        // Set same attribute value in adjacent ranges
        auto attributeID = SBAttributeRegistryGetAttributeID(registry, Attribute::Typeface);
        SBTextSetAttribute(text, 0, 1, attributeID, Typeface::Monospace);

        auto iterator = SBAttributeRunIteratorCreate(text);
        auto run = SBAttributeRunIteratorGetCurrent(iterator);

        assert(SBAttributeRunIteratorMoveNext(iterator));
        assert(run->index == 0);
        assert(run->length == 1);
        assert(run->attributeCount == 1);
        assert(run->attributeItems->attributeID == attributeID);
        assert(run->attributeItems->attributeValue == Typeface::Monospace);

        assert(!SBAttributeRunIteratorMoveNext(iterator));

        SBAttributeRunIteratorRelease(iterator);
        SBTextRelease(text);
    }

    // Test 2: Text with no attributes set
    {
        auto text = SBTextCreateTest("ABC");
        auto iterator = SBAttributeRunIteratorCreate(text);
        auto run = SBAttributeRunIteratorGetCurrent(iterator);

        assert(!SBAttributeRunIteratorMoveNext(iterator));
        assert(run->index == SBInvalidIndex);
        assert(run->length == 0);
        assert(run->attributeItems == nullptr);
        assert(run->attributeCount == 0);

        SBAttributeRunIteratorRelease(iterator);
        SBTextRelease(text);
    }

    // Test 3: Text with partial attribute coverage
    {
        auto text = SBTextCreateTest("ABC");
        auto registry = SBTextGetAttributeRegistry(text);

        auto attributeID = SBAttributeRegistryGetAttributeID(registry, Attribute::Typeface);
        SBTextSetAttribute(text, 1, 1, attributeID, Typeface::Serif);

        auto iterator = SBAttributeRunIteratorCreate(text);
        auto run = SBAttributeRunIteratorGetCurrent(iterator);

        assert(SBAttributeRunIteratorMoveNext(iterator));
        assert(run->index == 1);
        assert(run->length == 1);
        assert(run->attributeCount == 1);
        assert(run->attributeItems->attributeID == attributeID);
        assert(run->attributeItems->attributeValue == Typeface::Serif);

        assert(!SBAttributeRunIteratorMoveNext(iterator));

        SBAttributeRunIteratorRelease(iterator);
        SBTextRelease(text);
    }
}

void AttributeRunIteratorTests::testComplexScenarios() {
    auto text = SBTextCreateTest("ABCDEFGH");
    auto registry = SBTextGetAttributeRegistry(text);

    // Overlapping and adjacent runs
    auto attributeID = SBAttributeRegistryGetAttributeID(registry, Attribute::Typeface);
    SBTextSetAttribute(text, 0, 3, attributeID, Typeface::Serif);
    SBTextSetAttribute(text, 2, 3, attributeID, Typeface::SansSerif);
    SBTextSetAttribute(text, 4, 4, attributeID, Typeface::Monospace);

    auto iterator = SBAttributeRunIteratorCreate(text);
    auto run = SBAttributeRunIteratorGetCurrent(iterator);

    // Verify the resulting runs
    SBUInteger lastOffset = 0;
    while (SBAttributeRunIteratorMoveNext(iterator)) {
        SBUInteger offset = run->index;
        SBUInteger length = run->length;

        assert(offset >= lastOffset);
        assert(offset + length <= 8);

        lastOffset = offset + length;
    }

    // Verify all attributed text was covered
    assert(lastOffset <= 8);

    SBAttributeRunIteratorRelease(iterator);
    SBTextRelease(text);
}

#ifdef STANDALONE_TESTING

int main(int argc, const char *argv[]) {
    AttributeRunIteratorTests attributeRunIteratorTests;
    attributeRunIteratorTests.run();

    return 0;
}

#endif
