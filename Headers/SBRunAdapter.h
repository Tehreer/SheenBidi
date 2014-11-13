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

#ifndef SB_RUN_ADAPTER_H
#define SB_RUN_ADAPTER_H

#include "SBTypes.h"
#include "SBLine.h"

struct __SBRunAdapter;
typedef struct __SBRunAdapter SBRunAdapter;
typedef SBRunAdapter *SBRunAdapterRef;

/**
 * A structure containing the information about a bidi run.
 */
typedef struct {
    SBUInteger offset;  /**< The offset of a run from the start of a paragraph. */
    SBUInteger length;  /**< The length of a run. */
    SBLevel level;      /**< The bidi level of a run. */
} SBRunAgent;
typedef SBRunAgent *SBRunAgentRef;

/**
 * Creates a run adapter object.
 * @return
 *      A reference to a run adapter object.
 */
SBRunAdapterRef SBRunAdapterCreate(void);
/**
 * Loads a line in the adapter so that its runs can be obtained.
 * @param adapter
 *      The adapter in which the line will be loaded.
 * @param line
 *      The line which will be loaded in the adapter.
 */
void SBRunAdapterLoadLine(SBRunAdapterRef adapter, SBLineRef line);

/**
 * Provides the agent containing the information of current run.
 * @param adapter
 *      The adapter whose agent you want to obtain.
 */
const SBRunAgentRef SBRunAdapterGetAgent(SBRunAdapterRef adapter);
/**
 * Instructs the adapter to fetch information of next run from the loaded line.
 * @param adapter
 *      The adapter whom you want to instruct.
 * @return
 *      SBTrue if another run is available, SBFalse otherwise.
 * @note
 *      The adapter will be reset after fetching information of last run.
 */
SBBoolean SBRunAdapterMoveNext(SBRunAdapterRef adapter);
/**
 * Instructs the adapter to reset itself so that runs of the loaded line can be
 * obatained from the beginning.
 * @param adapter
 *      The adapter whom you want to reset.
 */
void SBRunAdapterReset(SBRunAdapterRef adapter);

SBRunAdapterRef SBRunAdapterRetain(SBRunAdapterRef adapter);
void SBRunAdapterRelease(SBRunAdapterRef adapter);

#endif
