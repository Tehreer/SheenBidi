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
#include <string>
#include <vector>

#include <SheenBidi/SBBase.h>
#include <SheenBidi/SBScript.h>

#include "Utilities/Convert.h"

#include "ScriptTests.h"

using namespace std;
using namespace SheenBidi;
using namespace SheenBidi::Utilities;

ScriptTests::ScriptTests()
{
}

void ScriptTests::run() {
    testGetUnicodeTag();
}

void ScriptTests::testGetUnicodeTag() {
    for (size_t script = 0; script <= UINT8_MAX; script++) {
        auto tag = SBScriptGetUnicodeTag(script);
        auto &alias = Convert::scriptToString(script);

        if (tag == 0) {
            assert(alias.empty());
        } else {
            assert(tag == Convert::stringToTag(alias));
        }
    }
}

#ifdef STANDALONE_TESTING

int main(int argc, const char *argv[]) {
    ScriptTests scriptTests;
    scriptTests.run();

    return 0;
}

#endif
