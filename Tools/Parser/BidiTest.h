/*
 * Copyright (C) 2015 Muhammad Tayyab Akram
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

#ifndef SHEENBIDI_PARSER_BIDI_TEST_H
#define SHEENBIDI_PARSER_BIDI_TEST_H

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace SheenBidi {
namespace Parser {

class BidiTest {
public:
    static const uint8_t LEVEL_X = UINT8_MAX;

    enum ParagraphDirection {
        Auto = 1,
        LTR = 2,
        RTL = 4,
    };

    struct TestCase {
        ParagraphDirection directions;
        std::vector<std::string> types;
        std::vector<uint8_t> levels;
        std::vector<size_t> order;
    };

    BidiTest(const std::string &directory);
    ~BidiTest();

    const TestCase &testCase() const;

    bool fetchNext();
    void reset();

private:
    std::ifstream m_stream;
    TestCase m_testCase;
};

}
}

#endif
