/*
 * Copyright (C) 2014 Muhammad Tayyab Akram
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

#include <stddef.h>
#include <stdlib.h>

#include <SBTypes.h>

#include "SBLine.h"
#include "SBRunAdapter.h"

SBRunAdapterRef SBRunAdapterCreate(void) {
    SBRunAdapterRef adapter;

    adapter = malloc(sizeof(SBRunAdapter));
    adapter->_retainCount = 1;
    adapter->_line = NULL;
    SBRunAdapterReset(adapter);

    return adapter;
}

void SBRunAdapterLoadLine(SBRunAdapterRef adapter, SBLineRef line) {
    SBLineRelease(adapter->_line);
    adapter->_line = SBLineRetain(line);
}

const SBRunAgentRef SBRunAdapterGetAgent(SBRunAdapterRef adapter) {
    return &adapter->agent;
}

SBBoolean SBRunAdapterMoveNext(SBRunAdapterRef adapter) {
    SBLineRef line = adapter->_line;

    if (adapter->_index < line->runCount) {
        SBRunRef run = &line->fixedRuns[adapter->_index];
        adapter->agent.offset = run->offset;
        adapter->agent.length = run->length;
        adapter->agent.level = run->level;
        adapter->_index += 1;

        return SBTrue;
    }

    SBRunAdapterReset(adapter);

    return SBFalse;
}

void SBRunAdapterReset(SBRunAdapterRef adapter) {
    adapter->_index = 0;
    adapter->agent.offset = SBInvalidLevel;
    adapter->agent.length = 0;
    adapter->agent.level = SBInvalidLevel;
}

SBRunAdapterRef SBRunAdapterRetain(SBRunAdapterRef adapter) {
    if (adapter) {
        ++adapter->_retainCount;
    }

    return adapter;
}

void SBRunAdapterRelease(SBRunAdapterRef adapter) {
    if (adapter && --adapter->_retainCount == 0) {
        SBLineRelease(adapter->_line);
        free(adapter);
    }
}
