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

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

#include <SheenBidi/SBAttributeInfo.h>
#include <SheenBidi/SBAttributeList.h>
#include <SheenBidi/SBAttributeRegistry.h>
#include <SheenBidi/SBScript.h>
#include <SheenBidi/SBText.h>
#include <SheenBidi/SBTextConfig.h>

extern "C" {
#include <API/SBBase.h>
#include <API/SBTextIterators.h>
}

#include "UniformRunIteratorTests.h"

using namespace std;
using namespace SheenBidi;

namespace {

using AttributeValue = uint32_t;

struct Typeface {
    static const AttributeValue Serif, SansSerif;
};

const AttributeValue Typeface::Serif = 1;
const AttributeValue Typeface::SansSerif = 2;

struct AttributeName {
    static constexpr auto Typeface = "typeface";
    static constexpr auto Color = "color";
};

const vector<SBAttributeInfo> TestAttributes = {
    {AttributeName::Typeface, 1, SBAttributeScopeCharacter},
    {AttributeName::Color, 2, SBAttributeScopeCharacter}
};

SBMutableTextRef SBTextCreateTest(const u16string &str, SBLevel baseLevel = SBLevelDefaultLTR) {
    auto registry = SBAttributeRegistryCreate(TestAttributes.data(), TestAttributes.size(),
        sizeof(AttributeValue), nullptr);

    auto config = SBTextConfigCreate();
    SBTextConfigSetAttributeRegistry(config, registry);
    SBTextConfigSetBaseLevel(config, baseLevel);

    auto text = SBTextCreateMutable(SBStringEncodingUTF16, config);
    SBTextAppendCodeUnits(text, str.data(), str.length());

    SBTextConfigRelease(config);
    SBAttributeRegistryRelease(registry);

    return text;
}

bool verifyAttributes(SBAttributeListRef attributes,
    const vector<pair<SBAttributeID, AttributeValue>> &items) {
    if (attributes == nullptr) {
        return false;
    }

    auto count = SBAttributeListGetCount(attributes);
    if (count != items.size()) {
        return false;
    }

    for (size_t i = 0; i < items.size(); ++i) {
        auto item = reinterpret_cast<const SBAttributeItem *>(SBAttributeListGetItem(attributes, i));
        auto value = *reinterpret_cast<const AttributeValue *>(item + 1);

        if (item->attributeID != items[i].first || value != items[i].second) {
            return false;
        }
    }

    return true;
}

}

void UniformRunIteratorTests::run() {
    testInitialization();
    testBasicIteration();
    testLevelBoundaries();
    testScriptBoundaries();
    testAttributeFiltering();
    testNoAttributesStillYieldsRun();
    testIntersectionOfBoundaries();
    testMultipleParagraphs();
    testPartialRange();
    testRetainRelease();
    testEdgeCases();
    testComplexScenarios();
}

void UniformRunIteratorTests::testInitialization() {
    // Test 1: Create iterator with valid text but zero-length
    {
        auto text = SBTextCreateTest(u"");
        auto iterator = SBTextCreateUniformRunIterator(text);
        assert(iterator != nullptr);

        auto run = SBUniformRunIteratorGetCurrent(iterator);
        assert(run != nullptr);
        assert(run->index == SBInvalidIndex);
        assert(run->length == 0);
        assert(run->level == 0);
        assert(run->script == SBScriptNil);
        assert(run->attributes == nullptr);

        assert(!SBUniformRunIteratorMoveNext(iterator));

        SBUniformRunIteratorRelease(iterator);
        SBTextRelease(text);
    }

    // Test 2: Create iterator with valid text and non-zero length
    {
        auto text = SBTextCreateTest(u"abc");
        auto iterator = SBTextCreateUniformRunIterator(text);
        assert(iterator != nullptr);

        auto run = SBUniformRunIteratorGetCurrent(iterator);
        assert(run != nullptr);
        assert(run->index == SBInvalidIndex);
        assert(run->length == 0);
        assert(run->level == 0);
        assert(run->script == SBScriptNil);
        assert(run->attributes == nullptr);

        SBUniformRunIteratorRelease(iterator);
        SBTextRelease(text);
    }
}

void UniformRunIteratorTests::testBasicIteration() {
    auto text = SBTextCreateTest(u"Hello");
    auto iterator = SBTextCreateUniformRunIterator(text);
    auto run = SBUniformRunIteratorGetCurrent(iterator);

    assert(SBUniformRunIteratorMoveNext(iterator));
    assert(run->index == 0);
    assert(run->length == 5);
    assert(run->level == 0);
    assert(run->script == SBScriptLATN);
    assert(run->attributes != nullptr);
    assert(SBAttributeListGetCount(run->attributes) == 0);

    assert(!SBUniformRunIteratorMoveNext(iterator));

    SBUniformRunIteratorRelease(iterator);
    SBTextRelease(text);
}

void UniformRunIteratorTests::testLevelBoundaries() {
    // Latin/Arabic/Latin: level and script boundaries coincide here.
    auto text = SBTextCreateTest(u"ABابCD");
    auto iterator = SBTextCreateUniformRunIterator(text);
    auto run = SBUniformRunIteratorGetCurrent(iterator);

    assert(SBUniformRunIteratorMoveNext(iterator));
    assert(run->index == 0);
    assert(run->length == 2);
    assert(run->level == 0);
    assert(run->script == SBScriptLATN);

    assert(SBUniformRunIteratorMoveNext(iterator));
    assert(run->index == 2);
    assert(run->length == 2);
    assert(run->level == 1);
    assert(run->script == SBScriptARAB);

    assert(SBUniformRunIteratorMoveNext(iterator));
    assert(run->index == 4);
    assert(run->length == 2);
    assert(run->level == 0);
    assert(run->script == SBScriptLATN);

    assert(!SBUniformRunIteratorMoveNext(iterator));

    SBUniformRunIteratorRelease(iterator);
    SBTextRelease(text);
}

void UniformRunIteratorTests::testScriptBoundaries() {
    // Latin followed by Devanagari, both resolving to level 0: only the script changes.
    auto text = SBTextCreateTest(u"ABकख");
    auto iterator = SBTextCreateUniformRunIterator(text);
    auto run = SBUniformRunIteratorGetCurrent(iterator);

    assert(SBUniformRunIteratorMoveNext(iterator));
    assert(run->index == 0);
    assert(run->length == 2);
    assert(run->level == 0);
    assert(run->script == SBScriptLATN);

    assert(SBUniformRunIteratorMoveNext(iterator));
    assert(run->index == 2);
    assert(run->length == 2);
    assert(run->level == 0);
    assert(run->script == SBScriptDEVA);

    assert(!SBUniformRunIteratorMoveNext(iterator));

    SBUniformRunIteratorRelease(iterator);
    SBTextRelease(text);
}

void UniformRunIteratorTests::testAttributeFiltering() {
    // Single script/level run throughout; only the attribute filter can introduce a boundary.
    auto text = SBTextCreateTest(u"ABCDEF");
    auto registry = SBTextGetAttributeRegistry(text);
    auto typeface = SBAttributeRegistryGetAttributeID(registry, AttributeName::Typeface);
    auto color = SBAttributeRegistryGetAttributeID(registry, AttributeName::Color);

    SBTextSetAttribute(text, 0, 3, typeface, &Typeface::Serif);
    SBTextSetAttribute(text, 3, 3, typeface, &Typeface::SansSerif);

    auto iterator = SBTextCreateUniformRunIterator(text);
    auto run = SBUniformRunIteratorGetCurrent(iterator);

    // Filtering on the typeface group must split at the attribute boundary.
    SBUniformRunIteratorSetupFilter(iterator, SBAttributeFilterMakeID(typeface));

    assert(SBUniformRunIteratorMoveNext(iterator));
    assert(run->index == 0);
    assert(run->length == 3);
    assert(verifyAttributes(run->attributes, {{typeface, Typeface::Serif}}));

    assert(SBUniformRunIteratorMoveNext(iterator));
    assert(run->index == 3);
    assert(run->length == 3);
    assert(verifyAttributes(run->attributes, {{typeface, Typeface::SansSerif}}));

    assert(!SBUniformRunIteratorMoveNext(iterator));

    // Filtering on a group that was never set must yield one empty-attribute run spanning
    // the whole text -- the typeface boundary is irrelevant to this filter.
    SBUniformRunIteratorSetupFilter(iterator,
        SBAttributeFilterMakeCollection(2, SBAttributeScopeCharacter));
    (void)color;

    assert(SBUniformRunIteratorMoveNext(iterator));
    assert(run->index == 0);
    assert(run->length == 6);
    assert(run->attributes != nullptr);
    assert(SBAttributeListGetCount(run->attributes) == 0);

    assert(!SBUniformRunIteratorMoveNext(iterator));

    SBUniformRunIteratorRelease(iterator);
    SBTextRelease(text);
}

void UniformRunIteratorTests::testNoAttributesStillYieldsRun() {
    // Unlike SBTextGetAttributes (whose caller naturally steps over a run with no matching
    // attributes), the uniform run iterator must still surface the run: level/script uniformity
    // is meaningful even when no attribute matches the configured filter.
    auto text = SBTextCreateTest(u"ABC");
    auto iterator = SBTextCreateUniformRunIterator(text);
    auto run = SBUniformRunIteratorGetCurrent(iterator);

    assert(SBUniformRunIteratorMoveNext(iterator));
    assert(run->index == 0);
    assert(run->length == 3);
    assert(run->level == 0);
    assert(run->script == SBScriptLATN);
    assert(run->attributes != nullptr);
    assert(SBAttributeListGetCount(run->attributes) == 0);

    assert(!SBUniformRunIteratorMoveNext(iterator));

    SBUniformRunIteratorRelease(iterator);
    SBTextRelease(text);
}

void UniformRunIteratorTests::testIntersectionOfBoundaries() {
    // "Hello " + Hebrew "SHALOM" + " world". Level/script alone would produce:
    //   {0,6,L0,LATN} {6,4,L1,HEBR} {10,1,L0,HEBR} {11,5,L0,LATN}
    // (the trailing space merges into the Hebrew script run but resolves to level 0). Placing a
    // typeface change at index 8 -- strictly inside the {6,4,L1,HEBR} run -- must split that run
    // in two without disturbing any other boundary.
    const char16_t codeUnits[] = {
        u'H', u'e', u'l', u'l', u'o', u' ',
        0x05E9, 0x05DC, 0x05D5, 0x05DD,
        u' ', u'w', u'o', u'r', u'l', u'd'
    };
    auto text = SBTextCreateTest(u16string(codeUnits, 16));
    auto registry = SBTextGetAttributeRegistry(text);
    auto typeface = SBAttributeRegistryGetAttributeID(registry, AttributeName::Typeface);

    SBTextSetAttribute(text, 0, 8, typeface, &Typeface::Serif);
    SBTextSetAttribute(text, 8, 8, typeface, &Typeface::SansSerif);

    auto iterator = SBTextCreateUniformRunIterator(text);
    SBUniformRunIteratorSetupFilter(iterator, SBAttributeFilterMakeID(typeface));
    auto run = SBUniformRunIteratorGetCurrent(iterator);

    struct Expected {
        SBUInteger index;
        SBUInteger length;
        SBLevel level;
        SBScript script;
        AttributeValue typefaceValue;
    };
    const vector<Expected> result = {
        {0,  6, 0, SBScriptLATN, Typeface::Serif},
        {6,  2, 1, SBScriptHEBR, Typeface::Serif},
        {8,  2, 1, SBScriptHEBR, Typeface::SansSerif},
        {10, 1, 0, SBScriptHEBR, Typeface::SansSerif},
        {11, 5, 0, SBScriptLATN, Typeface::SansSerif},
    };

    size_t index = 0;
    while (SBUniformRunIteratorMoveNext(iterator)) {
        assert(index < result.size());
        assert(run->index == result[index].index);
        assert(run->length == result[index].length);
        assert(run->level == result[index].level);
        assert(run->script == result[index].script);
        assert(verifyAttributes(run->attributes, {{typeface, result[index].typefaceValue}}));

        index += 1;
    }

    assert(index == result.size());

    SBUniformRunIteratorRelease(iterator);
    SBTextRelease(text);
}

void UniformRunIteratorTests::testMultipleParagraphs() {
    // Both paragraphs are pure Latin at level 0 with no attributes -- level, script, and
    // attributes are all identical across the paragraph break, but the break must still force
    // two separate runs.
    auto text = SBTextCreateTest(u"AB\nCD");
    auto iterator = SBTextCreateUniformRunIterator(text);
    auto run = SBUniformRunIteratorGetCurrent(iterator);

    assert(SBUniformRunIteratorMoveNext(iterator));
    assert(run->index == 0);
    assert(run->length == 3);
    assert(run->level == 0);

    assert(SBUniformRunIteratorMoveNext(iterator));
    assert(run->index == 3);
    assert(run->length == 2);
    assert(run->level == 0);

    assert(!SBUniformRunIteratorMoveNext(iterator));

    SBUniformRunIteratorRelease(iterator);
    SBTextRelease(text);
}

void UniformRunIteratorTests::testPartialRange() {
    auto text = SBTextCreateTest(u"ABCDEF");
    auto iterator = SBTextCreateUniformRunIterator(text);
    auto run = SBUniformRunIteratorGetCurrent(iterator);

    SBUniformRunIteratorReset(iterator, 2, 3);

    assert(SBUniformRunIteratorMoveNext(iterator));
    assert(run->index == 2);
    assert(run->length == 3);
    assert(run->level == 0);
    assert(run->script == SBScriptLATN);

    assert(!SBUniformRunIteratorMoveNext(iterator));

    SBUniformRunIteratorRelease(iterator);
    SBTextRelease(text);
}

void UniformRunIteratorTests::testRetainRelease() {
    auto text = SBTextCreateTest(u"ABC");
    auto iterator = SBTextCreateUniformRunIterator(text);

    auto retained = SBUniformRunIteratorRetain(iterator);
    assert(retained == iterator);

    SBUniformRunIteratorRelease(retained);
    SBUniformRunIteratorRelease(iterator);
    SBTextRelease(text);
}

void UniformRunIteratorTests::testEdgeCases() {
    // Test 1: Single character text
    {
        auto text = SBTextCreateTest(u"A");
        auto iterator = SBTextCreateUniformRunIterator(text);
        auto run = SBUniformRunIteratorGetCurrent(iterator);

        assert(SBUniformRunIteratorMoveNext(iterator));
        assert(run->index == 0);
        assert(run->length == 1);
        assert(run->level == 0);
        assert(run->script == SBScriptLATN);

        assert(!SBUniformRunIteratorMoveNext(iterator));

        SBUniformRunIteratorRelease(iterator);
        SBTextRelease(text);
    }

    // Test 2: Text with only Common-script characters
    {
        auto text = SBTextCreateTest(u"123");
        auto iterator = SBTextCreateUniformRunIterator(text);
        auto run = SBUniformRunIteratorGetCurrent(iterator);

        assert(SBUniformRunIteratorMoveNext(iterator));
        assert(run->index == 0);
        assert(run->length == 3);
        assert(run->level == 0);
        assert(run->script == SBScriptZYYY);

        assert(!SBUniformRunIteratorMoveNext(iterator));

        SBUniformRunIteratorRelease(iterator);
        SBTextRelease(text);
    }

    // Test 3: Iterate immediately after Create, without an explicit Reset call -- the default
    // range (the whole text, as set at creation time) must already be in effect.
    {
        auto text = SBTextCreateTest(u"XYZ");
        auto iterator = SBTextCreateUniformRunIterator(text);
        auto run = SBUniformRunIteratorGetCurrent(iterator);

        assert(SBUniformRunIteratorMoveNext(iterator));
        assert(run->index == 0);
        assert(run->length == 3);

        assert(!SBUniformRunIteratorMoveNext(iterator));

        SBUniformRunIteratorRelease(iterator);
        SBTextRelease(text);
    }
}

void UniformRunIteratorTests::testComplexScenarios() {
    // Two paragraphs; the first mixes Latin/Hebrew/Latin with a typeface change, the second is
    // plain Latin. Verifies paragraph breaks, level/script boundaries, and attribute boundaries
    // all combine correctly across a multi-paragraph text.
    const char16_t codeUnits[] = {
        u'A', u'B',                     // Latin (para 0)
        0x05D0, 0x05D1,                 // Hebrew (para 0)
        u'C', u'D', u'\n',              // Latin + paragraph separator (para 0)
        u'E', u'F'                      // Latin (para 1)
    };
    auto text = SBTextCreateTest(u16string(codeUnits, 9));
    auto registry = SBTextGetAttributeRegistry(text);
    auto typeface = SBAttributeRegistryGetAttributeID(registry, AttributeName::Typeface);

    // Split the trailing Latin run inside the first paragraph via an attribute change.
    SBTextSetAttribute(text, 0, 5, typeface, &Typeface::Serif);
    SBTextSetAttribute(text, 5, 4, typeface, &Typeface::SansSerif);

    auto iterator = SBTextCreateUniformRunIterator(text);
    SBUniformRunIteratorSetupFilter(iterator, SBAttributeFilterMakeID(typeface));
    auto run = SBUniformRunIteratorGetCurrent(iterator);

    struct Expected {
        SBUInteger index;
        SBUInteger length;
        SBLevel level;
        SBScript script;
        AttributeValue typefaceValue;
    };
    const vector<Expected> result = {
        {0, 2, 0, SBScriptLATN, Typeface::Serif},
        {2, 2, 1, SBScriptHEBR, Typeface::Serif},
        {4, 1, 0, SBScriptLATN, Typeface::Serif},
        {5, 2, 0, SBScriptLATN, Typeface::SansSerif},
        {7, 2, 0, SBScriptLATN, Typeface::SansSerif},
    };

    size_t index = 0;
    while (SBUniformRunIteratorMoveNext(iterator)) {
        assert(index < result.size());
        assert(run->index == result[index].index);
        assert(run->length == result[index].length);
        assert(run->level == result[index].level);
        assert(run->script == result[index].script);
        assert(verifyAttributes(run->attributes, {{typeface, result[index].typefaceValue}}));

        index += 1;
    }

    assert(index == result.size());

    SBUniformRunIteratorRelease(iterator);
    SBTextRelease(text);
}

#ifdef STANDALONE_TESTING

int main(int argc, const char *argv[]) {
    UniformRunIteratorTests uniformRunIteratorTests;
    uniformRunIteratorTests.run();

    return 0;
}

#endif
