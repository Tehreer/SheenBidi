/*
 * Copyright (C) 2018 Muhammad Tayyab Akram
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

extern "C" {
#include <Headers/SBBase.h>
#include <Headers/SBCodepointSequence.h>
#include <Headers/SBScriptLocator.h>
}

#include <cassert>
#include <string>
#include <vector>

#include "ScriptLocatorTester.h"

using namespace std;
using namespace SheenBidi::Tester;

struct run {
    SBUInteger start;
    SBUInteger end;
    SBScript script;

    bool operator ==(const run& other) const {
        return start == other.start
            && end == other.end
            && script == other.script;
    }
};

static void u32Test(const u32string string, const vector<run> runs)
{
    SBCodepointSequence sequence;
    sequence.stringEncoding = SBStringEncodingUTF32;
    sequence.stringBuffer = (void *)&string[0];
    sequence.stringLength = string.length();

    SBScriptLocatorRef locator = SBScriptLocatorCreate();
    SBScriptAgentRef agent = SBScriptLocatorGetAgent(locator);

    SBScriptLocatorLoadCodepoints(locator, &sequence);

    vector<run> output;
    while (SBScriptLocatorMoveNext(locator)) {
        output.push_back({agent->offset, agent->offset + agent->length, agent->script});
    }

    SBScriptLocatorRelease(locator);

    assert(runs == output);
}

ScriptLocatorTester::ScriptLocatorTester()
{
}

void ScriptLocatorTester::test()
{
    /* Test with an empty string. */
    u32Test(U"", { });

    /* Test with a single script in ascii, UTF-16 and UTF-32 ranges. */
    u32Test(U"Script", { {0, 6, SBScriptLATN} });
    u32Test(U"تحریر", { {0, 5, SBScriptARAB} });
    u32Test(U"\U0001D84C\U0001D84D\U0001D84E\U0001D84F\U0001D850\U0001D851\U0001D852", { {0, 7, SBScriptSGNW} });
}
