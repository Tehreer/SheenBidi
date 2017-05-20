/*
 * Copyright (C) 2016 Muhammad Tayyab Akram
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

#ifndef SHEENBIDI_TESTER_ALGORITHM_TESTER_H
#define SHEENBIDI_TESTER_ALGORITHM_TESTER_H

#include <cstdint>
#include <vector>

extern "C" {
#include <SheenBidi.h>
}

#include <Parser/BidiTest.h>
#include <Parser/BidiCharacterTest.h>
#include <Parser/BidiMirroring.h>

namespace SheenBidi {
namespace Tester {

class AlgorithmTester {
public:
    AlgorithmTester(Parser::BidiTest *bidiTest, Parser::BidiCharacterTest *bidiBracketTest, Parser::BidiMirroring *bidiMirroring);
    ~AlgorithmTester();

    void testAlgorithm();
    void testMulticharNewline();
    void test();

private:
    Parser::BidiTest *m_bidiTest;
    Parser::BidiCharacterTest *m_bidiCharacterTest;
    Parser::BidiMirroring *m_bidiMirroring;

    size_t testCounter;
    size_t failCounter;

    SBMirrorLocatorRef m_mirrorLocator;

    SBCodepoint m_genChars[256];
    SBUInteger m_charCount;

    SBLevel m_inputLevel;
    uint8_t m_paragraphLevel;

    SBUInteger m_runCount;
    const SBRun *m_runArray;

    uint32_t m_genMirrors[256];
    size_t m_mirrorCount;

    const std::vector<uint8_t> *m_levels;
    const std::vector<size_t> *m_order;

    void loadCharacters(const std::vector<std::string> &types);
    void loadCharacters(const std::vector<uint32_t> &codePoints);
    void loadMirrors();

    void displayBidiTestCase() const;
    void displayBidiCharacterTestCase() const;

    bool testLevels() const;
    bool testOrder() const;
    bool testMirrors() const;

    bool conductTest();
    void analyzeBidiTest();
    void analyzeBidiCharacterTest();
};

}
}

#endif
