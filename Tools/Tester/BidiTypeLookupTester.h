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

#ifndef _SHEENBIDI__TESTER__BIDI_TYPE_LOOKUP_TESTER_H
#define _SHEENBIDI__TESTER__BIDI_TYPE_LOOKUP_TESTER_H

#include <Parser/DerivedBidiClass.h>

namespace SheenBidi {
namespace Tester {

class BidiTypeLookupTester {
public:
    BidiTypeLookupTester(const Parser::DerivedBidiClass &derivedBidiClass);

    void test();

private:
    const Parser::DerivedBidiClass &m_derivedBidiClass;
};

}
}

#endif
