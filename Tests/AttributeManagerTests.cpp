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
#include <cstddef>
#include <random>
#include <string>
#include <vector>

#include <SheenBidi/SBBase.h>

extern "C" {
#include <Source/AttributeDictionary.h>
#include <Source/AttributeManager.h>
#include "Source/SBAttributeRegistry.h"
#include <Source/SBText.h>
}

#include "AttributeManagerTests.h"

using namespace std;
using namespace SheenBidi;

class Number {
public:
    static Number *create(uint8_t value) {
        return new Number(value);
    }

    static SBBoolean equal(const void *first, const void *second) {
        auto number1 = static_cast<const Number *>(first);
        auto number2 = static_cast<const Number *>(second);

        return number1->m_value == number2->m_value;
    }

    static const void *retain(const void *value) {
        auto number = static_cast<Number *>(const_cast<void *>(value));
        number->m_retainCount += 1;
        return number;
    }

    static void release(const void *value) {
        auto number = static_cast<Number *>(const_cast<void *>(value));
        if (--number->m_retainCount == 0) {
            delete number;
        }
    }

    static auto callbacks() {
        return SBAttributeCallbacks {
            equal, retain, release
        };
    }

private:
    const uint8_t m_value;
    size_t m_retainCount = 1;

    explicit Number(uint8_t value) : m_value(value) { }
};

class AutoNumber {
public:
    explicit AutoNumber(uint8_t value)
        : m_ptr(Number::create(value)) { }

    ~AutoNumber() {
        Number::release(m_ptr);
    }

    operator const void *() const { return m_ptr; }

private:
    const Number *m_ptr;
};

struct Attribute {
    static constexpr auto Alignment = "alignment";
    static constexpr auto Color = "color";
    static constexpr auto Font = "font";
    static constexpr auto Language = "language";
};

struct Alignment {
    static auto leading() { return AutoNumber(11); }
    static auto center() { return AutoNumber(12); }
    static auto training() { return AutoNumber(13); }
};

struct Color {
    static auto red() { return AutoNumber(21); }
    static auto green() { return AutoNumber(22); }
    static auto blue() { return AutoNumber(23); }
    static auto yellow() { return AutoNumber(24); }
    static auto black() { return AutoNumber(25); }
    static auto purple() { return AutoNumber(26); }
};

struct Font {
    static auto arial() { return AutoNumber(31); }
    static auto times() { return AutoNumber(32); }
};

struct Language {
    static auto english() { return AutoNumber(41); }
    static auto urdu() { return AutoNumber(42); }
};

static const vector<SBAttributeInfo> TestAttributes = {
    {Attribute::Color, 1, SBAttributeScopeCharacter, Number::callbacks()},
    {Attribute::Font, 1, SBAttributeScopeCharacter, Number::callbacks()},
    {Attribute::Language, 2, SBAttributeScopeParagraph, Number::callbacks()},
    {Attribute::Alignment, 3, SBAttributeScopeParagraph, Number::callbacks()}
};

// Helper to generate random strings
static string generateString(size_t length) {
    static const string characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    static random_device rd;
    static mt19937 generator(rd());
    static uniform_int_distribution<> distribution(0, characters.size() - 1);

    string result;
    result.reserve(length);
    for (size_t i = 0; i < length; i++) {
        result += characters[distribution(generator)];
    }
    return result;
}

// Helper to verify attribute at position
static bool verifyAttribute(AttributeManagerRef manager, size_t index,
    SBAttributeID attributeID, const void *expectedValue)
{
    auto entry = AttributeManagerFindEntry(manager, index, nullptr, nullptr);
    auto item = AttributeDictionaryFindItem(entry->attributes, attributeID);
    if (!item) {
        return expectedValue == nullptr;
    }

    return SBAttributeRegistryIsEqualAttribute(manager->_registry, attributeID,
        item->attributeValue, expectedValue);
}

static SBMutableTextRef SBTextCreateWithRegistry(SBAttributeRegistryRef registry) {
    auto config = SBTextConfigCreate();
    SBTextConfigSetAttributeRegistry(config, registry);

    auto text = SBTextCreateMutable(SBStringEncodingUTF8, config);

    SBTextConfigRelease(config);

    return text;
}

static SBMutableTextRef SBTextCreateWithDefaultRegistry() {
    auto registry = SBAttributeRegistryCreate(TestAttributes.data(), TestAttributes.size());
    auto text = SBTextCreateWithRegistry(registry);

    SBAttributeRegistryRelease(text->attributeRegistry);

    return text;
}

static void SBTextAppendRandomCodeUnits(SBMutableTextRef text, SBUInteger length) {
    const auto content = generateString(length);
    SBTextAppendCodeUnits(text, content.c_str(), length);
}

static void SBTextInsertRandomParagraph(SBMutableTextRef text,
    SBUInteger index, SBUInteger length) {
    const auto paragraph = generateString(length - 1) + '\n';
    SBTextInsertCodeUnits(text, index, paragraph.c_str(), length);
}

static void SBTextReplaceWithRandomCodeUnits(SBMutableTextRef text,
    SBUInteger index, SBUInteger length, SBUInteger newLength) {
    const auto content = generateString(newLength);
    SBTextReplaceCodeUnits(text, index, length, content.c_str(), newLength);
}

void AttributeManagerTests::run() {
    testInitializeWithNullRegistry();
    testInitializeWithValidRegistry();

    // Set/Remove attribute tests
    testSetCharacterAttribute();
    testSetParagraphAttributeExtension();
    testRemoveAttribute();
    testMultipleAttributesSameRange();
    testOverwriteAttributeValue();
    testComplexAttributeOverlapping();
    testBoundaryConditions();
    testLargeTextOperations();
    testMultipleParagraphAttributes();
    testAttributeRemovalEdgeCases();
    testOperationsWithNullRegistry();
    testZeroLengthOperations();

    // Replacement tests
    testReplaceRangePureInsertion();
    testReplaceRangePureDeletion();
    testReplaceRangeBasicReplacement();
    testReplaceRangeAtTextBoundaries();
    testReplaceRangeWithSingleEntry();
    testReplaceRangeSpanningMultipleEntries();
    testReplaceRangeRemovingEntireEntries();
    testReplaceRangeWithParagraphAttributes();
    testReplaceRangeParagraphMerging();
    testReplaceRangeComplexAttributePatterns();
    testReplaceRangeMultipleAttributes();
    testReplaceRangeAttributeInheritance();

    // Run iteration tests
    testGetRunByIDBasic();
    testGetRunByFilteredCollection();
    testEmptyRuns();
    testFilterByAttributeGroup();
    testRunBoundariesWithMixedAttributes();
    testComplexRunDetectionScenarios();
}

void AttributeManagerTests::testInitializeWithNullRegistry() {
    auto text = SBTextCreateWithRegistry(nullptr);
    auto manager = &text->attributeManager;

    assert(manager->_registry == nullptr);
    assert(manager->parent == text);

    SBTextRelease(text);
}

void AttributeManagerTests::testInitializeWithValidRegistry() {
    auto text = SBTextCreateWithDefaultRegistry();
    auto manager = &text->attributeManager;

    assert(manager->_registry != nullptr);
    assert(manager->_registry == text->attributeRegistry);
    assert(manager->parent == text);
    assert(manager->_entries.count == 1);

    SBTextAppendRandomCodeUnits(text, 5);
    assert(manager->_entries.count == 1);

    // All positions should have no attributes initially
    for (SBUInteger i = 0; i < 5; i++) {
        assert(verifyAttribute(manager, i, 0, nullptr));
    }

    SBTextRelease(text);
}

void AttributeManagerTests::testSetCharacterAttribute() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 5);
    SBTextAppendRandomCodeUnits(text, 5);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);

    SBTextSetAttribute(text, 1, 3, colorID, Color::blue());

    // Verify attributes in range
    for (size_t i = 1; i < 4; i++) {
        assert(verifyAttribute(manager, i, colorID, Color::blue()));
    }

    // Verify position 0 doesn't have the attribute
    assert(verifyAttribute(manager, 0, colorID, nullptr));

    SBTextRelease(text);
}

void AttributeManagerTests::testSetParagraphAttributeExtension() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextInsertRandomParagraph(text, 0, 10);
    SBTextInsertRandomParagraph(text, 10, 10);
    SBTextInsertRandomParagraph(text, 20, 10);

    auto manager = &text->attributeManager;
    auto langID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Language);

    // Set the paragraph attribute in the middle of the first paragraph
    SBTextSetAttribute(text, 6, 5, langID, Language::english());

    // Should extend to the entire first and second paragraphs (0-19)
    for (size_t i = 0; i < 20; i++) {
        assert(verifyAttribute(manager, i, langID, Language::english()));
    }

    // The third paragraph should not have the attribute
    assert(verifyAttribute(manager, 20, langID, nullptr));

    SBTextRelease(text);
}

void AttributeManagerTests::testRemoveAttribute() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 5);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);

    // Set and verify attribute
    SBTextSetAttribute(text, 1, 3, colorID, Color::green());
    assert(verifyAttribute(manager, 2, colorID, Color::green()));

    // Remove attribute
    SBTextRemoveAttribute(text, 2, 1, colorID);
    assert(verifyAttribute(manager, 2, colorID, nullptr));

    SBTextRelease(text);
}

void AttributeManagerTests::testMultipleAttributesSameRange() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 5);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);
    auto fontID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Font);

    // Set multiple attributes at the same position
    SBTextSetAttribute(text, 2, 1, colorID, Color::purple());
    SBTextSetAttribute(text, 2, 1, fontID, Font::arial());

    assert(verifyAttribute(manager, 2, colorID, Color::purple()));
    assert(verifyAttribute(manager, 2, fontID, Font::arial()));

    SBTextRelease(text);
}

void AttributeManagerTests::testOverwriteAttributeValue() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 3);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);

    // Set initial value
    SBTextSetAttribute(text, 1, 1, colorID, Color::red());
    assert(verifyAttribute(manager, 1, colorID, Color::red()));

    // Overwrite with new value
    SBTextSetAttribute(text, 1, 1, colorID, Color::black());
    assert(verifyAttribute(manager, 1, colorID, Color::black()));

    SBTextRelease(text);
}

void AttributeManagerTests::testComplexAttributeOverlapping() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextInsertRandomParagraph(text, 0, 20);
    SBTextInsertRandomParagraph(text, 20, 20);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);
    auto fontID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Font);

    // Create overlapping attribute regions
    SBTextSetAttribute(text, 0, 10, colorID, Color::red());
    SBTextSetAttribute(text, 5, 10, fontID, Font::arial());
    SBTextSetAttribute(text, 10, 10, colorID, Color::blue());

    // Verify overlapping regions
    for (size_t i = 0; i < 5; i++) {
        assert(verifyAttribute(manager, i, colorID, Color::red()));
        assert(verifyAttribute(manager, i, fontID, nullptr));
    }

    for (size_t i = 5; i < 10; i++) {
        assert(verifyAttribute(manager, i, colorID, Color::red()));
        assert(verifyAttribute(manager, i, fontID, Font::arial()));
    }

    for (size_t i = 10; i < 15; i++) {
        assert(verifyAttribute(manager, i, colorID, Color::blue()));
        assert(verifyAttribute(manager, i, fontID, Font::arial()));
    }

    for (size_t i = 15; i < 20; i++) {
        assert(verifyAttribute(manager, i, colorID, Color::blue()));
        assert(verifyAttribute(manager, i, fontID, nullptr));
    }

    SBTextRelease(text);
}

void AttributeManagerTests::testBoundaryConditions() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextInsertRandomParagraph(text, 0, 10);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);

    // Test at the start boundary
    SBTextSetAttribute(text, 0, 1, colorID, Color::red());
    assert(verifyAttribute(manager, 0, colorID, Color::red()));

    // Test at the end boundary
    SBTextSetAttribute(text, 9, 1, colorID, Color::blue());
    assert(verifyAttribute(manager, 9, colorID, Color::blue()));

    // Test a single character in the middle
    SBTextSetAttribute(text, 5, 1, colorID, Color::green());
    assert(verifyAttribute(manager, 5, colorID, Color::green()));

    // Test zero-length range (should not crash)
    SBTextSetAttribute(text, 5, 0, colorID, Color::yellow());

    SBTextRelease(text);
}

void AttributeManagerTests::testLargeTextOperations() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 1000);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);

    // Set attributes in chunks
    SBTextSetAttribute(text, 0, 250, colorID, Color::red());
    SBTextSetAttribute(text, 250, 250, colorID, Color::green());
    SBTextSetAttribute(text, 500, 250, colorID, Color::blue());
    SBTextSetAttribute(text, 750, 250, colorID, Color::yellow());

    // Verify chunks
    assert(verifyAttribute(manager, 0, colorID, Color::red()));
    assert(verifyAttribute(manager, 249, colorID, Color::red()));
    assert(verifyAttribute(manager, 250, colorID, Color::green()));
    assert(verifyAttribute(manager, 499, colorID, Color::green()));
    assert(verifyAttribute(manager, 500, colorID, Color::blue()));
    assert(verifyAttribute(manager, 749, colorID, Color::blue()));
    assert(verifyAttribute(manager, 750, colorID, Color::yellow()));
    assert(verifyAttribute(manager, 999, colorID, Color::yellow()));

    SBTextRelease(text);
}

void AttributeManagerTests::testMultipleParagraphAttributes() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextInsertRandomParagraph(text, 0, 10);
    SBTextInsertRandomParagraph(text, 10, 10);
    SBTextInsertRandomParagraph(text, 20, 10);
    SBTextInsertRandomParagraph(text, 30, 10);

    auto manager = &text->attributeManager;
    auto langID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Language);
    auto alignID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Alignment);

    // Set different paragraph attributes
    SBTextSetAttribute(text, 5, 1, langID, Language::english());
    SBTextSetAttribute(text, 15, 1, langID, Language::urdu());
    SBTextSetAttribute(text, 25, 1, alignID, Alignment::center());

    // Verify paragraph-level extension
    for (size_t i = 0; i < 10; i++) {
        assert(verifyAttribute(manager, i, langID, Language::english()));
    }
    for (size_t i = 10; i < 20; i++) {
        assert(verifyAttribute(manager, i, langID, Language::urdu()));
    }
    for (size_t i = 20; i < 30; i++) {
        assert(verifyAttribute(manager, i, alignID, Alignment::center()));
    }

    SBTextRelease(text);
}

void AttributeManagerTests::testAttributeRemovalEdgeCases() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextInsertRandomParagraph(text, 0, 10);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);

    // Set attribute
    SBTextSetAttribute(text, 0, 10, colorID, Color::red());

    // Remove from middle
    SBTextRemoveAttribute(text, 3, 4, colorID);

    // Verify boundaries
    assert(verifyAttribute(manager, 0, colorID, Color::red()));
    assert(verifyAttribute(manager, 2, colorID, Color::red()));
    assert(verifyAttribute(manager, 3, colorID, nullptr));
    assert(verifyAttribute(manager, 6, colorID, nullptr));
    assert(verifyAttribute(manager, 7, colorID, Color::red()));
    assert(verifyAttribute(manager, 9, colorID, Color::red()));

    // Remove from start
    SBTextRemoveAttribute(text, 0, 3, colorID);
    assert(verifyAttribute(manager, 0, colorID, nullptr));
    assert(verifyAttribute(manager, 2, colorID, nullptr));

    // Remove from end
    SBTextRemoveAttribute(text, 7, 3, colorID);
    assert(verifyAttribute(manager, 7, colorID, nullptr));
    assert(verifyAttribute(manager, 9, colorID, nullptr));

    SBTextRelease(text);
}

void AttributeManagerTests::testOperationsWithNullRegistry() {
    auto text = SBTextCreateWithRegistry(nullptr);
    SBTextAppendRandomCodeUnits(text, 10);

    // These should handle null registry gracefully
    SBTextDeleteCodeUnits(text, 0, 5);
    SBTextRemoveAttribute(text, 0, 5, 1);

    // Setting attributes with null registry should not crash
    SBTextSetAttribute(text, 0, 1, 1, "value");

    SBTextRelease(text);
}

void AttributeManagerTests::testZeroLengthOperations() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 5);

    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);

    // Zero-length operations should be handled without crashing
    SBTextSetAttribute(text, 2, 0, colorID, Color::red());
    SBTextRemoveAttribute(text, 2, 0, colorID);

    SBTextRelease(text);
}

void AttributeManagerTests::testReplaceRangePureInsertion() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 10);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);

    // Set up initial attributes: [RRRRRBBBBB] where R=red, B=blue
    SBTextSetAttribute(text, 0, 5, colorID, Color::red());
    SBTextSetAttribute(text, 5, 5, colorID, Color::blue());

    // Pure insertion at position 3 (middle of red range)
    SBTextReplaceWithRandomCodeUnits(text, 3, 0, 3);

    // Expected: [RRR + new3 + RRBBBBB] where new3 inherits red
    assert(verifyAttribute(manager, 0, colorID, Color::red()));   // First red preserved
    assert(verifyAttribute(manager, 2, colorID, Color::red()));   // First red preserved
    assert(verifyAttribute(manager, 3, colorID, Color::red()));   // New text inherits red
    assert(verifyAttribute(manager, 5, colorID, Color::red()));   // New text inherits red
    assert(verifyAttribute(manager, 6, colorID, Color::red()));   // Second part of red
    assert(verifyAttribute(manager, 7, colorID, Color::red()));   // Second part of red
    assert(verifyAttribute(manager, 8, colorID, Color::blue()));  // Blue range shifted
    assert(verifyAttribute(manager, 12, colorID, Color::blue())); // Blue range shifted

    SBTextRelease(text);
}

void AttributeManagerTests::testReplaceRangePureDeletion() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 10);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);

    // Set up initial attributes: [RRRGGGBBB] where R=red, G=green, B=blue
    SBTextSetAttribute(text, 0, 3, colorID, Color::red());
    SBTextSetAttribute(text, 3, 3, colorID, Color::green());
    SBTextSetAttribute(text, 6, 3, colorID, Color::blue());

    // Pure deletion of range 2-7 (overlapping all three ranges)
    SBTextReplaceWithRandomCodeUnits(text, 2, 5, 0);

    // Expected: [RRBB] - deletion removes parts of each range
    assert(verifyAttribute(manager, 0, colorID, Color::red()));   // First part of red preserved
    assert(verifyAttribute(manager, 1, colorID, Color::red()));   // First part of red preserved
    assert(verifyAttribute(manager, 2, colorID, Color::blue()));  // Blue range shifted
    assert(verifyAttribute(manager, 3, colorID, Color::blue()));  // Blue range shifted

    SBTextRelease(text);
}

void AttributeManagerTests::testReplaceRangeBasicReplacement() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 8);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);

    // Set up initial attributes: [RRRRGGGG] where R=red, G=green
    SBTextSetAttribute(text, 0, 4, colorID, Color::red());
    SBTextSetAttribute(text, 4, 4, colorID, Color::green());

    // Replace range 2-6 with 5 new characters
    SBTextReplaceWithRandomCodeUnits(text, 2, 4, 5);

    // Expected: [RR + new5 + GG] where new5 inherits red then green
    assert(verifyAttribute(manager, 0, colorID, Color::red()));   // First red preserved
    assert(verifyAttribute(manager, 1, colorID, Color::red()));   // First red preserved
    assert(verifyAttribute(manager, 2, colorID, Color::red()));   // New text inherits red
    assert(verifyAttribute(manager, 6, colorID, Color::red()));   // New text inherits red
    assert(verifyAttribute(manager, 7, colorID, Color::green())); // Switch to green
    assert(verifyAttribute(manager, 8, colorID, Color::green())); // Remaining green

    SBTextRelease(text);
}

void AttributeManagerTests::testReplaceRangeAtTextBoundaries() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 5);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);

    // Set attributes: first 3 red, last 2 blue
    SBTextSetAttribute(text, 0, 3, colorID, Color::red());
    SBTextSetAttribute(text, 3, 2, colorID, Color::blue());

    // Test replacement at start boundary
    SBTextReplaceWithRandomCodeUnits(text, 0, 2, 4);
    assert(verifyAttribute(manager, 0, colorID, Color::red()));   // New text inherits red
    assert(verifyAttribute(manager, 3, colorID, Color::red()));   // New text inherits red
    assert(verifyAttribute(manager, 4, colorID, Color::red()));   // Old text keeps red
    assert(verifyAttribute(manager, 5, colorID, Color::blue()));  // Blue range shifted

    // Test replacement at end boundary (appending)
    SBTextReplaceWithRandomCodeUnits(text, 7, 0, 3);
    for (SBUInteger i = 5; i < 10; i++) {
        assert(verifyAttribute(manager, i, colorID, Color::blue()));
    }

    SBTextRelease(text);
}

void AttributeManagerTests::testReplaceRangeWithSingleEntry() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 5);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);

    // Single entry covers everything (red)
    SBTextSetAttribute(text, 0, 5, colorID, Color::red());

    // Various replacements should maintain single entry
    SBTextReplaceWithRandomCodeUnits(text, 2, 1, 2); // Replace middle
    assert(manager->_entries.count == 1);
    for (SBUInteger i = 0; i < 6; i++) {
        assert(verifyAttribute(manager, i, colorID, Color::red()));
    }

    SBTextReplaceWithRandomCodeUnits(text, 0, 1, 1); // Replace start
    assert(manager->_entries.count == 1);

    SBTextReplaceWithRandomCodeUnits(text, 5, 1, 1); // Replace end
    assert(manager->_entries.count == 1);

    SBTextRelease(text);
}

void AttributeManagerTests::testReplaceRangeSpanningMultipleEntries() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 12);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);

    // Create multiple attribute entries: [RRR GGGG BBBBB]
    SBTextSetAttribute(text, 0, 3, colorID, Color::red());
    SBTextSetAttribute(text, 3, 4, colorID, Color::green());
    SBTextSetAttribute(text, 7, 5, colorID, Color::blue());

    // Replace range spanning all entries (2-9)
    SBTextReplaceWithRandomCodeUnits(text, 2, 7, 6);

    // Verify resulting attributes preserve the pattern
    assert(verifyAttribute(manager, 0, colorID, Color::red()));   // First red preserved
    assert(verifyAttribute(manager, 1, colorID, Color::red()));   // First red preserved
    assert(verifyAttribute(manager, 2, colorID, Color::red()));   // New text inherits red
    assert(verifyAttribute(manager, 3, colorID, Color::red()));   // New text inherits red
    assert(verifyAttribute(manager, 7, colorID, Color::red()));   // New text inherits red
    assert(verifyAttribute(manager, 8, colorID, Color::blue()));  // Remaining blue
    assert(verifyAttribute(manager, 10, colorID, Color::blue())); // Remaining blue

    SBTextRelease(text);
}

void AttributeManagerTests::testReplaceRangeRemovingEntireEntries() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 9);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);

    // Create entries: [RRR GGG BBB]
    SBTextSetAttribute(text, 0, 3, colorID, Color::red());
    SBTextSetAttribute(text, 3, 3, colorID, Color::green());
    SBTextSetAttribute(text, 6, 3, colorID, Color::blue());

    // Replace entire middle entry (positions 3-5)
    SBTextReplaceWithRandomCodeUnits(text, 3, 3, 2);

    // New text should inherit from first replaced character (green)
    assert(verifyAttribute(manager, 0, colorID, Color::red()));   // Red preserved
    assert(verifyAttribute(manager, 2, colorID, Color::red()));   // Red preserved
    assert(verifyAttribute(manager, 3, colorID, Color::green())); // New text gets green
    assert(verifyAttribute(manager, 4, colorID, Color::green())); // New text gets green
    assert(verifyAttribute(manager, 5, colorID, Color::blue()));  // Blue preserved
    assert(verifyAttribute(manager, 7, colorID, Color::blue()));  // Blue preserved

    SBTextRelease(text);
}

void AttributeManagerTests::testReplaceRangeWithParagraphAttributes() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextInsertRandomParagraph(text, 0, 6);  // First paragraph
    SBTextInsertRandomParagraph(text, 6, 6);  // Second paragraph

    auto manager = &text->attributeManager;
    auto langID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Language);

    // Set different paragraph attributes
    SBTextSetAttribute(text, 0, 1, langID, Language::english()); // First paragraph
    SBTextSetAttribute(text, 6, 1, langID, Language::urdu());    // Second paragraph

    // Replace text within first paragraph (should not affect paragraph scope)
    SBTextReplaceWithRandomCodeUnits(text, 2, 2, 3);

    // Paragraph attributes should still cover entire paragraphs
    for (SBUInteger i = 0; i < 7; i++) { // First paragraph extended due to insertion
        assert(verifyAttribute(manager, i, langID, Language::english()));
    }
    for (SBUInteger i = 7; i < 13; i++) { // Second paragraph
        assert(verifyAttribute(manager, i, langID, Language::urdu()));
    }

    SBTextRelease(text);
}

void AttributeManagerTests::testReplaceRangeParagraphMerging() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextInsertRandomParagraph(text, 0, 6);  // "12345\n"
    SBTextInsertRandomParagraph(text, 6, 6);  // "67890\n"

    auto manager = &text->attributeManager;
    auto langID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Language);
    auto alignID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Alignment);

    // Set different paragraph attributes
    SBTextSetAttribute(text, 0, 1, langID, Language::english());
    SBTextSetAttribute(text, 0, 1, alignID, Alignment::leading());
    SBTextSetAttribute(text, 6, 1, langID, Language::urdu());
    SBTextSetAttribute(text, 6, 1, alignID, Alignment::center());

    // Delete the newline character that separates paragraphs
    SBTextReplaceWithRandomCodeUnits(text, 5, 2, 0); // Delete newline and first char

    // Paragraphs should merge with combined attributes
    for (SBUInteger i = 0; i < 10; i++) {
        // Both language and alignment attributes should be present
        assert(verifyAttribute(manager, i, langID, Language::english()));
        assert(verifyAttribute(manager, i, alignID, Alignment::leading()));
    }

    SBTextRelease(text);
}

void AttributeManagerTests::testReplaceRangeComplexAttributePatterns() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 15);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);
    auto fontID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Font);

    // Complex pattern: [RR FF RR+FF GG BB]
    SBTextSetAttribute(text, 0, 2, colorID, Color::red());
    SBTextSetAttribute(text, 2, 2, fontID, Font::arial());
    SBTextSetAttribute(text, 4, 3, colorID, Color::red());
    SBTextSetAttribute(text, 4, 3, fontID, Font::arial());
    SBTextSetAttribute(text, 7, 3, colorID, Color::green());
    SBTextSetAttribute(text, 10, 5, colorID, Color::blue());

    // Complex replacement overlapping multiple regions
    SBTextReplaceWithRandomCodeUnits(text, 3, 8, 5);

    // Verify the complex result maintains attribute boundaries
    assert(verifyAttribute(manager, 0, colorID, Color::red()));
    assert(verifyAttribute(manager, 1, colorID, Color::red()));
    assert(verifyAttribute(manager, 2, fontID, Font::arial()));
    assert(verifyAttribute(manager, 7, fontID, Font::arial()));
    assert(verifyAttribute(manager, 8, colorID, Color::blue()));
    assert(verifyAttribute(manager, 11, colorID, Color::blue()));

    SBTextRelease(text);
}

void AttributeManagerTests::testReplaceRangeMultipleAttributes() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 10);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);
    auto fontID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Font);

    // Set multiple attributes on same range
    SBTextSetAttribute(text, 0, 10, colorID, Color::red());
    SBTextSetAttribute(text, 3, 4, fontID, Font::times());

    // Replace range that overlaps font attribute
    SBTextReplaceWithRandomCodeUnits(text, 4, 2, 3);

    // Both attributes should be preserved
    for (size_t i = 0; i < 11; i++) {
        assert(verifyAttribute(manager, i, colorID, Color::red()));
    }
    for (size_t i = 3; i < 8; i++) {
        assert(verifyAttribute(manager, 3, fontID, Font::times()));
    }

    SBTextRelease(text);
}

void AttributeManagerTests::testReplaceRangeAttributeInheritance() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 8);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);

    // Test that inserted text inherits correct attributes
    SBTextSetAttribute(text, 0, 4, colorID, Color::red());
    SBTextSetAttribute(text, 4, 4, colorID, Color::blue());

    // Insert at boundary between red and blue
    SBTextReplaceWithRandomCodeUnits(text, 4, 0, 2);

    // New text should inherit from left side (red)
    for (size_t i = 0; i < 6; i++) {
        assert(verifyAttribute(manager, i, colorID, Color::red()));
    }

    // Insert at start of blue range with deletion
    SBTextReplaceWithRandomCodeUnits(text, 6, 2, 3);
    for (size_t i = 6; i < 11; i++) {
        assert(verifyAttribute(manager, i, colorID, Color::blue()));
    }

    SBTextRelease(text);
}

void AttributeManagerTests::testReplaceRangeEdgeCases() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 5);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);

    SBTextSetAttribute(text, 0, 5, colorID, Color::red());

    // Edge case: replace entire text
    SBTextReplaceWithRandomCodeUnits(text, 0, 5, 3);
    for (SBUInteger i = 0; i < 3; i++) {
        assert(verifyAttribute(manager, i, colorID, Color::red()));
    }

    // Edge case: replace with same length
    SBTextReplaceWithRandomCodeUnits(text, 1, 1, 1);
    assert(verifyAttribute(manager, 1, colorID, Color::red()));

    // Edge case: replace zero characters at end (should not crash)
    SBTextReplaceWithRandomCodeUnits(text, 3, 0, 0);

    SBTextRelease(text);
}

void AttributeManagerTests::testReplaceRangeZeroLengthOperations() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 5);

    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);

    // Zero-length operations should be handled without crashing
    SBTextReplaceWithRandomCodeUnits(text, 2, 0, 0); // No-op
    SBTextReplaceWithRandomCodeUnits(text, 0, 0, 0); // No-op at start
    SBTextReplaceWithRandomCodeUnits(text, 5, 0, 0); // No-op at end

    // Setting attributes after zero-length ops should work
    SBTextSetAttribute(text, 2, 1, colorID, Color::red());
    assert(verifyAttribute(&text->attributeManager, 2, colorID, Color::red()));

    SBTextRelease(text);
}

void AttributeManagerTests::testGetRunByIDBasic() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextInsertRandomParagraph(text, 0, 10);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);

    // Set attribute on first half
    SBTextSetAttribute(text, 0, 5, colorID, Color::purple());

    AttributeDictionary output;
    AttributeDictionaryInitialize(&output);

    SBUInteger startIndex = 0;
    SBUInteger endIndex = 10;

    // First run (0-4 with color)
    assert(AttributeManagerGetOnwardRunByFilteringID(manager,
        &startIndex, endIndex, colorID, &output));
    assert(output._list.count == 1);
    assert(startIndex == 5);

    // Second run (5-9 without color)
    assert(AttributeManagerGetOnwardRunByFilteringID(manager,
        &startIndex, endIndex, colorID, &output));
    assert(output._list.count == 0);
    assert(startIndex == 10);

    AttributeDictionaryFinalize(&output, nullptr);
    SBTextRelease(text);
}

void AttributeManagerTests::testGetRunByFilteredCollection() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextInsertRandomParagraph(text, 0, 10);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);
    auto fontID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Font);

    // Set different attributes in different ranges
    SBTextSetAttribute(text, 0, 3, colorID, Color::red());
    SBTextSetAttribute(text, 0, 3, fontID, Font::arial());
    SBTextSetAttribute(text, 3, 4, colorID, Color::blue());

    AttributeDictionary output;
    AttributeDictionaryInitialize(&output);

    SBUInteger startIndex = 0;
    SBUInteger endIndex = 10;

    // Get runs filtered by character scope
    assert(AttributeManagerGetOnwardRunByFilteringCollection(manager,
        &startIndex, endIndex, SBAttributeScopeCharacter, SBAttributeGroupNone, &output));

    assert(output._list.count == 2); // color and font
    assert(startIndex == 3);

    AttributeDictionaryFinalize(&output, nullptr);
    SBTextRelease(text);
}

void AttributeManagerTests::testEmptyRuns() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 5);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);

    AttributeDictionary output;
    AttributeDictionaryInitialize(&output);

    SBUInteger startIndex = 0;
    SBUInteger endIndex = 5;

    // Should find a run with no attributes
    assert(AttributeManagerGetOnwardRunByFilteringID(manager,
        &startIndex, endIndex, colorID, &output));
    assert(output._list.count == 0);
    assert(startIndex == 5);

    AttributeDictionaryFinalize(&output, nullptr);
    SBTextRelease(text);
}

void AttributeManagerTests::testFilterByAttributeGroup() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 10);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);
    auto fontID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Font);
    auto langID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Language);

    // Set attributes from different groups
    SBTextSetAttribute(text, 0, 5, colorID, Color::red());
    SBTextSetAttribute(text, 0, 5, fontID, Font::arial());
    SBTextSetAttribute(text, 3, 5, langID, Language::english());

    AttributeDictionary output;
    AttributeDictionaryInitialize(&output);

    SBUInteger startIndex = 0;
    SBUInteger endIndex = 10;

    // Filter by character group (group 1)
    assert(AttributeManagerGetOnwardRunByFilteringCollection(manager,
        &startIndex, endIndex, SBAttributeScopeCharacter, 1, &output));

    // Should only get color and font (both group 1), not language (group 2)
    assert(startIndex == 5);
    assert(output._list.count == 2);

    AttributeDictionaryFinalize(&output, nullptr);
    SBTextRelease(text);
}

void AttributeManagerTests::testRunBoundariesWithMixedAttributes() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextInsertRandomParagraph(text, 0, 30);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);
    auto fontID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Font);

    // Create a pattern: color-only, both, font-only, none
    SBTextSetAttribute(text, 0, 5, colorID, Color::red());
    SBTextSetAttribute(text, 5, 5, colorID, Color::red());
    SBTextSetAttribute(text, 5, 5, fontID, Font::arial());
    SBTextSetAttribute(text, 10, 5, fontID, Font::arial());
    // 15-30: no attributes

    AttributeDictionary output;
    AttributeDictionaryInitialize(&output);

    SBUInteger startIndex = 0;
    SBUInteger endIndex = 30;

    // Get runs for the color
    assert(AttributeManagerGetOnwardRunByFilteringID(manager,
        &startIndex, endIndex, colorID, &output));
    assert(startIndex == 10); // color ends at 10

    // Continue to the next run (no color)
    assert(AttributeManagerGetOnwardRunByFilteringID(manager,
        &startIndex, endIndex, colorID, &output));
    assert(startIndex == 30); // no color from 10-30

    AttributeDictionaryFinalize(&output, nullptr);
    SBTextRelease(text);
}

void AttributeManagerTests::testComplexRunDetectionScenarios() {
    auto text = SBTextCreateWithDefaultRegistry();
    SBTextAppendRandomCodeUnits(text, 20);

    auto manager = &text->attributeManager;
    auto colorID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Color);
    auto fontID = SBAttributeRegistryGetAttributeID(text->attributeRegistry, Attribute::Font);

    // Create a complex pattern: A-B-A-B where A=color, B=font, AB=both
    SBTextSetAttribute(text, 0, 5, colorID, Color::red());      // 0-4: color
    SBTextSetAttribute(text, 5, 5, fontID, Font::arial());     // 5-9: font
    SBTextSetAttribute(text, 10, 5, colorID, Color::blue());    // 10-14: color
    SBTextSetAttribute(text, 10, 5, fontID, Font::arial());    // 10-14: both
    SBTextSetAttribute(text, 15, 5, fontID, Font::times());    // 15-19: font

    AttributeDictionary output;
    AttributeDictionaryInitialize(&output);

    SBUInteger startIndex = 0;

    // Test run detection with mixed attributes
    AttributeManagerGetOnwardRunByFilteringCollection(manager,
        &startIndex, 20, SBAttributeScopeCharacter, SBAttributeGroupNone, &output);
    assert(startIndex == 5); // First run: positions 0-4 (color only)

    AttributeManagerGetOnwardRunByFilteringCollection(manager,
        &startIndex, 20, SBAttributeScopeCharacter, SBAttributeGroupNone, &output);
    assert(startIndex == 10); // Second run: positions 5-9 (font only)

    AttributeManagerGetOnwardRunByFilteringCollection(manager,
        &startIndex, 20, SBAttributeScopeCharacter, SBAttributeGroupNone, &output);
    assert(startIndex == 15); // Third run: positions 10-14 (both color and font)

    AttributeManagerGetOnwardRunByFilteringCollection(manager,
        &startIndex, 20, SBAttributeScopeCharacter, SBAttributeGroupNone, &output);
    assert(startIndex == 20); // Fourth run: positions 15-19 (font only)

    AttributeDictionaryFinalize(&output, nullptr);
    SBTextRelease(text);
}

#ifdef STANDALONE_TESTING

int main(int argc, const char *argv[]) {
    AttributeManagerTests attributeStorageTests;
    attributeStorageTests.run();

    return 0;
}

#endif
