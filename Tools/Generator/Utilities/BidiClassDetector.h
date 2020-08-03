/*
 * Copyright (C) 2015-2020 Muhammad Tayyab Akram
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

#ifndef SHEENBIDI_GENERATOR_UTILITIES_BIDI_CLASS_H
#define SHEENBIDI_GENERATOR_UTILITIES_BIDI_CLASS_H

#include <cstdint>
#include <map>
#include <string>

#include <Parser/DerivedBidiClass.h>

namespace SheenBidi {
namespace Generator {
namespace Utilities {

class BidiClassDetector {
public:
    BidiClassDetector(const Parser::DerivedBidiClass &derivedBidiClass);
    
    uint8_t numberForCodePoint(uint32_t codePoint) const;
    const std::string &nameForCodePoint(uint32_t codePoint) const;

    const std::string &numberToName(uint8_t number) const;
    const uint8_t nameToNumber(const std::string &name) const;

private:
    const Parser::DerivedBidiClass &m_derivedBidiClass;
    std::vector<uint8_t> m_numbers;

    std::vector<std::string> m_numberToName;
    std::map<std::string, uint8_t> m_nameToNumber;
};

}
}
}

#endif
