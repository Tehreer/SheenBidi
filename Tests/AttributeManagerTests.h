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

#ifndef _SHEENBIDI__ATTRIBUTE_MANAGER_TESTS_H
#define _SHEENBIDI__ATTRIBUTE_MANAGER_TESTS_H

namespace SheenBidi {

class AttributeManagerTests {
public:
    AttributeManagerTests() = default;

    void run();

private:
    static void testInitializeWithNullRegistry();
    static void testInitializeWithValidRegistry();
    static void testAttributePropagation();
    static void testSetCharacterAttribute();
    static void testSetParagraphAttributeExtension();
    static void testRemoveAttribute();
    static void testRemoveRangeWithAttributes();
    static void testParagraphMergingOnRemove();
    static void testGetRunByIDBasic();
    static void testGetRunByFilteredCollection();
    static void testFilterByAttributeGroup();
    static void testComplexRunDetectionScenarios();
    static void testMultipleAttributesSameRange();
    static void testOverwriteAttributeValue();
    static void testEmptyRuns();
    static void testComplexAttributeOverlapping();
    static void testBoundaryConditions();
    static void testLargeTextOperations();
    static void testMultipleParagraphAttributes();
    static void testRunBoundariesWithMixedAttributes();
    static void testAttributeRemovalEdgeCases();
    static void testOperationsWithNullRegistry();
    static void testZeroLengthOperations();
};

}

#endif
