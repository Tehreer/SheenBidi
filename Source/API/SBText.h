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

#ifndef _SB_INTERNAL_TEXT_H
#define _SB_INTERNAL_TEXT_H

#include <API/SBBase.h>

#if SB_TEXT_API_SUPPORTED

#include <SheenBidi/SBAttributeRegistry.h>
#include <SheenBidi/SBText.h>

#include <Core/Object.h>
#include <Text/AttributeManager.h>
#include <Text/BidiTypesBuffer.h>
#include <Text/TextAnalysis.h>
#include <Text/TextBuffer.h>

/**
 * A thin façade composing the code-unit buffer, the derived bidi types, the paragraph/script
 * analysis, and the attribute overlay of a text object. Mutation entry points delegate to these
 * layers rather than manipulating code units, bidi types, and paragraphs inline.
 */
typedef struct _SBText {
    ObjectBase _base;
    SBBoolean isMutable;
    SBBoolean isEditing;
    SBAttributeRegistryRef attributeRegistry;
    TextBuffer buffer;
    BidiTypesBuffer bidiTypes;
    TextAnalysis analysis;
    AttributeManager attributeManager;
} SBText;

/**
 * Creates a mutable text object with explicit parameters.
 *
 * @param encoding
 *      String encoding.
 * @param attributeRegistry
 *      Attribute registry (can be `NULL`).
 * @param baseLevel
 *      Base bidirectional level.
 * @return
 *      New mutable text object, or `NULL` on failure.
 */
SB_INTERNAL SBMutableTextRef SBTextCreateMutableWithParameters(SBStringEncoding encoding,
    SBAttributeRegistryRef attributeRegistry, SBLevel baseLevel);

#endif

#endif
