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

#include <stddef.h>
#include <stdlib.h>

#include "SBLine.h"
#include "SBTypes.h"
#include "SBRunAdapter.h"

static void _SBRunAdapterUnload(SBRunAdapterRef adapter);

SBRunAdapterRef SBRunAdapterCreate(void)
{
    SBRunAdapterRef adapter;

    adapter = malloc(sizeof(SBRunAdapter));
    adapter->_paragraph = NULL;
    adapter->_line = NULL;
    adapter->_type = _SBRunAdapterTypeNone;
    adapter->_retainCount = 1;
    SBRunAdapterReset(adapter);

    return adapter;
}

static void _SBRunAdapterUnload(SBRunAdapterRef adapter)
{
    SBParagraphRelease(adapter->_paragraph);
    SBLineRelease(adapter->_line);

    adapter->_paragraph = NULL;
    adapter->_line = NULL;
}

void SBRunAdapterLoadParagraph(SBRunAdapterRef adapter, SBParagraphRef paragraph)
{
    _SBRunAdapterUnload(adapter);

    adapter->_paragraph = SBParagraphRetain(paragraph);
    adapter->_type = _SBRunAdapterTypeParagraph;

    SBRunAdapterReset(adapter);
}

void SBRunAdapterLoadLine(SBRunAdapterRef adapter, SBLineRef line)
{
    _SBRunAdapterUnload(adapter);

    adapter->_line = SBLineRetain(line);
    adapter->_type = _SBRunAdapterTypeLine;

    SBRunAdapterReset(adapter);
}

SBRunAgentRef SBRunAdapterGetAgent(SBRunAdapterRef adapter)
{
    return &adapter->agent;
}

SBBoolean SBRunAdapterMoveNext(SBRunAdapterRef adapter)
{
    switch (adapter->_type) {
    case _SBRunAdapterTypeParagraph:
        {
            SBParagraphRef paragraph = adapter->_paragraph;
            SBUInteger nextIndex = adapter->_index;

            if (nextIndex < paragraph->length) {
                SBLevel *allLevels = paragraph->fixedLevels;
                SBLevel runLevel = allLevels[nextIndex];

                while (++nextIndex < paragraph->length) {
                    if (allLevels[nextIndex] != runLevel) {
                        break;
                    }
                }

                adapter->agent.offset = adapter->_index;
                adapter->agent.length = nextIndex - adapter->_index;
                adapter->agent.level = runLevel;
                adapter->_index = nextIndex;

                return SBTrue;
            }

            SBRunAdapterReset(adapter);
        }
        break;

    case _SBRunAdapterTypeLine:
        {
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
        }
        break;
    }

    return SBFalse;
}

void SBRunAdapterReset(SBRunAdapterRef adapter)
{
    adapter->_index = 0;
    adapter->agent.offset = SBInvalidIndex;
    adapter->agent.length = 0;
    adapter->agent.level = SBInvalidLevel;
}

SBRunAdapterRef SBRunAdapterRetain(SBRunAdapterRef adapter)
{
    if (adapter) {
        ++adapter->_retainCount;
    }

    return adapter;
}

void SBRunAdapterRelease(SBRunAdapterRef adapter)
{
    if (adapter && --adapter->_retainCount == 0) {
        SBLineRelease(adapter->_line);
        free(adapter);
    }
}
