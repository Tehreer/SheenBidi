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

#ifndef SHEENBIDI_PARSER_BIDI_BRACKETS_H
#define SHEENBIDI_PARSER_BIDI_BRACKETS_H

#include <cstdint>
#include <string>
#include <vector>

#include "UnicodeVersion.h"

namespace SheenBidi {
namespace Parser {

class BidiBrackets {
public:
    BidiBrackets(const std::string &directory);
    ~BidiBrackets();

    uint32_t firstCodePoint() const;
    uint32_t lastCodePoint() const;

    UnicodeVersion &version() const;
    uint32_t pairedBracketForCodePoint(uint32_t) const;
    char pairedBracketTypeForCodePoint(uint32_t) const;

private:
    uint32_t m_firstCodePoint;
    uint32_t m_lastCodePoint;

    UnicodeVersion *m_version;
    std::vector<uint32_t> m_pairedBrackets;
    std::vector<char> m_pairedBracketTypes;
};

}
}

#endif
