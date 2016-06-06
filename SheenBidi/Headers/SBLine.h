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

#ifndef _SB_PUBLIC_LINE_H
#define _SB_PUBLIC_LINE_H

#include "SBBase.h"
#include "SBCodepointSequence.h"
#include "SBRun.h"

typedef struct _SBLine *SBLineRef;

/**
 * Provides the offset of the line in original string.
 * @param line
 *      The line whose offset you want to obtain.
 * @return
 *      The offset of the line passed in.
 */
SBUInteger SBLineGetOffset(SBLineRef line);

/**
 * Provides the length of the line in original string.
 * @param line
 *      The line whose length you want to obtain.
 * @return
 *      The length of the line passed in.
 */
SBUInteger SBLineGetLength(SBLineRef line);

/**
 * Provides the total run count of the line.
 * @param line
 *      The line whose run count you want to obtain.
 * @return
 *      The total runs of the line passed in.
 */
SBUInteger SBLineGetRunCount(SBLineRef line);

/**
 * Provides a direct pointer for the run array stored in the line.
 * @param line
 *      The line from which to access runs.
 * @return
 *      A valid pointer to an array of SBRun structures. 
 */
const SBRun *SBLineGetRunsPtr(SBLineRef line);

SBLineRef SBLineRetain(SBLineRef line);
void SBLineRelease(SBLineRef line);

#endif
