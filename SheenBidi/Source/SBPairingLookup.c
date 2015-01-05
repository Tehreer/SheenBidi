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

/*
 * The data for this file has been taken from BidiBrackets.txt and
 * BidiMirroring.txt of unicode version 7.0
 */

#include <SBConfig.h>
#include <SBTypes.h>

#include "SBBracketType.h"
#include "SBPairingLookup.h"

const SBInt16 __SBPairDifferences[] = {
    0,     1,     -1,    2,     -2,    16,    -16,   3,     -3,    2016,  138,
    1824,  2104,  2108,  2106,  -138,  8,     7,     -8,    -7,    -1824, -2016,
    -2104, -2106, -2108
};

const SBUInt8 __SBPairData[] = {
#define __SBPairData_000 0x000
    65,  130, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   3,   0,   4,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   67,  0,   132, 0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   67,  0,   132, 0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   5,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   6,   0,   0,
    0,   0,
#define __SBPairData_098 0x098
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,
#define __SBPairData_130 0x130
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   65,  130,
    65,  130, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,
#define __SBPairData_1C8 0x1C8
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   65,  130, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,
#define __SBPairData_260 0x260
    0,   1,   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   65,  130,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   65,  130, 0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   65,  130, 0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,
#define __SBPairData_2F8 0x2F8
    0,   0,   0,   0,   0,   0,   0,   0,   7,   7,   7,   8,   8,   8,   0,
    0,   0,   0,   0,   0,   0,   9,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    1,   2,   0,   0,   0,   0,   0,   10,  0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   1,   2,   1,   2,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   2,   1,   2,   1,
    2,   1,   2,   0,   0,   1,   2,   1,   2,   1,   2,   1,   2,   1,   2,
    1,   2,   1,   2,   1,   2,   1,   2,   1,   2,   1,   2,   1,   2,   1,
    2,   1,   2,   1,   2,   0,   0,   0,   1,   2,   1,   2,   0,   0,   0,
    0,   0,
#define __SBPairData_390 0x390
    11,  0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   2,   0,   0,   12,
    0,   13,  14,  0,   14,  0,   0,   0,   0,   1,   2,   1,   2,   1,   2,
    1,   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   1,   2,   1,   2,   15,  0,   0,   1,   2,   0,   0,
    0,   0,   1,   2,   1,   2,   1,   2,   1,   2,   1,   2,   1,   2,   1,
    2,   1,   2,   1,   2,   1,   2,   1,   2,   1,   2,   0,   0,   1,   2,
    16,  16,  16,  0,   17,  17,  0,   0,   18,  18,  18,  19,  19,  0,   0,
    0,   0,   0,   0,   0,   0,   0,   65,  130, 65,  130, 0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   65,  130, 0,   0,   0,
    0,   0,
#define __SBPairData_428 0x428
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   65,  130, 65,  130, 65,  130, 65,  130, 65,  130, 65,  130, 65,  130,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   1,   2,   65,  130, 0,   1,   2,   0,   3,   0,   4,   0,   0,
    0,   0,   0,   0,   0,   1,   2,   0,   0,   0,   0,   0,   0,   1,   2,
    0,   0,   0,   1,   2,   1,   2,   65,  130, 65,  130, 65,  130, 65,  130,
    65,  130,
#define __SBPairData_4C0 0x4C0
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   65,  130, 65,  130, 65,  130,
    65,  130, 65,  130, 71,  129, 66,  136, 65,  130, 65,  130, 65,  130, 65,
    130, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,
#define __SBPairData_558 0x558
    20,  0,   0,   0,   0,   0,   0,   0,   1,   2,   0,   0,   1,   2,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   1,   2,   1,   2,   0,   1,   2,
    0,   0,   65,  130, 65,  130, 0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   21,  0,   0,   1,   2,   0,   0,   65,  130, 0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   2,   1,   2,   0,
    0,   0,   0,   0,   1,   2,   0,   0,   0,   0,   0,   0,   1,   2,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,
#define __SBPairData_5F0 0x5F0
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   1,   2,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   2,   0,   0,
    1,   2,   1,   2,   1,   2,   1,   2,   0,   0,   0,   0,   0,   0,   1,
    2,   0,   0,   0,   0,   1,   2,   1,   2,   1,   2,   1,   2,   1,   2,
    1,   2,   0,   0,   0,   0,   1,   2,   0,   0,   0,   1,   2,   1,   2,
    1,   2,   1,   2,   0,   1,   2,   0,   0,   1,   2,   0,   0,   0,   0,
    0,   0,   1,   2,   1,   2,   1,   2,   1,   2,   1,   2,   1,   2,   0,
    0,   0,   0,   0,   0,   1,   2,   1,   2,   1,   2,   1,   2,   1,   2,
    0,   0,   0,   0,   0,   0,   0,   22,  0,   0,   0,   0,   23,  24,  23,
    0,   0,
#define __SBPairData_688 0x688
    0,   0,   0,   0,   1,   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    1,   2,   1,   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,
#define __SBPairData_720 0x720
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   1,   2,   1,   2,   0,   0,   0,   1,   2,   0,   1,
    2,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    1,   2,   0,   0,   1,   2,   65,  130, 65,  130, 65,  130, 65,  130, 0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,
#define __SBPairData_7B8 0x7B8
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   65,  130, 65,  130, 65,  130, 65,  130, 65,
    130, 0,   0,   65,  130, 65,  130, 65,  130, 65,  130, 0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,
#define __SBPairData_850 0x850
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   65,  130, 65,  130, 65,  130, 0,   0,   0,   0,   0,   1,   2,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,
#define __SBPairData_8E8 0x8E8
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   65,  130, 0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    3,   0,   4,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   67,  0,   132, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   67,  0,   132, 0,   65,  130, 0,   65,  130
};

const SBUInt16 __SBPairIndexes[] = {
    __SBPairData_000,             /**< %0028..00BF */
    __SBPairData_098,             /**< %00C0..0157 */
    __SBPairData_098,             /**< %0158..01EF */
    __SBPairData_098,             /**< %01F0..0287 */
    __SBPairData_098,             /**< %0288..031F */
    __SBPairData_098,             /**< %0320..03B7 */
    __SBPairData_098,             /**< %03B8..044F */
    __SBPairData_098,             /**< %0450..04E7 */
    __SBPairData_098,             /**< %04E8..057F */
    __SBPairData_098,             /**< %0580..0617 */
    __SBPairData_098,             /**< %0618..06AF */
    __SBPairData_098,             /**< %06B0..0747 */
    __SBPairData_098,             /**< %0748..07DF */
    __SBPairData_098,             /**< %07E0..0877 */
    __SBPairData_098,             /**< %0878..090F */
    __SBPairData_098,             /**< %0910..09A7 */
    __SBPairData_098,             /**< %09A8..0A3F */
    __SBPairData_098,             /**< %0A40..0AD7 */
    __SBPairData_098,             /**< %0AD8..0B6F */
    __SBPairData_098,             /**< %0B70..0C07 */
    __SBPairData_098,             /**< %0C08..0C9F */
    __SBPairData_098,             /**< %0CA0..0D37 */
    __SBPairData_098,             /**< %0D38..0DCF */
    __SBPairData_098,             /**< %0DD0..0E67 */
    __SBPairData_098,             /**< %0E68..0EFF */
    __SBPairData_130,             /**< %0F00..0F97 */
    __SBPairData_098,             /**< %0F98..102F */
    __SBPairData_098,             /**< %1030..10C7 */
    __SBPairData_098,             /**< %10C8..115F */
    __SBPairData_098,             /**< %1160..11F7 */
    __SBPairData_098,             /**< %11F8..128F */
    __SBPairData_098,             /**< %1290..1327 */
    __SBPairData_098,             /**< %1328..13BF */
    __SBPairData_098,             /**< %13C0..1457 */
    __SBPairData_098,             /**< %1458..14EF */
    __SBPairData_098,             /**< %14F0..1587 */
    __SBPairData_098,             /**< %1588..161F */
    __SBPairData_1C8,             /**< %1620..16B7 */
    __SBPairData_098,             /**< %16B8..174F */
    __SBPairData_098,             /**< %1750..17E7 */
    __SBPairData_098,             /**< %17E8..187F */
    __SBPairData_098,             /**< %1880..1917 */
    __SBPairData_098,             /**< %1918..19AF */
    __SBPairData_098,             /**< %19B0..1A47 */
    __SBPairData_098,             /**< %1A48..1ADF */
    __SBPairData_098,             /**< %1AE0..1B77 */
    __SBPairData_098,             /**< %1B78..1C0F */
    __SBPairData_098,             /**< %1C10..1CA7 */
    __SBPairData_098,             /**< %1CA8..1D3F */
    __SBPairData_098,             /**< %1D40..1DD7 */
    __SBPairData_098,             /**< %1DD8..1E6F */
    __SBPairData_098,             /**< %1E70..1F07 */
    __SBPairData_098,             /**< %1F08..1F9F */
    __SBPairData_098,             /**< %1FA0..2037 */
    __SBPairData_260,             /**< %2038..20CF */
    __SBPairData_098,             /**< %20D0..2167 */
    __SBPairData_098,             /**< %2168..21FF */
    __SBPairData_2F8,             /**< %2200..2297 */
    __SBPairData_390,             /**< %2298..232F */
    __SBPairData_098,             /**< %2330..23C7 */
    __SBPairData_098,             /**< %23C8..245F */
    __SBPairData_098,             /**< %2460..24F7 */
    __SBPairData_098,             /**< %24F8..258F */
    __SBPairData_098,             /**< %2590..2627 */
    __SBPairData_098,             /**< %2628..26BF */
    __SBPairData_098,             /**< %26C0..2757 */
    __SBPairData_428,             /**< %2758..27EF */
    __SBPairData_098,             /**< %27F0..2887 */
    __SBPairData_098,             /**< %2888..291F */
    __SBPairData_4C0,             /**< %2920..29B7 */
    __SBPairData_558,             /**< %29B8..2A4F */
    __SBPairData_5F0,             /**< %2A50..2AE7 */
    __SBPairData_688,             /**< %2AE8..2B7F */
    __SBPairData_098,             /**< %2B80..2C17 */
    __SBPairData_098,             /**< %2C18..2CAF */
    __SBPairData_098,             /**< %2CB0..2D47 */
    __SBPairData_098,             /**< %2D48..2DDF */
    __SBPairData_720,             /**< %2DE0..2E77 */
    __SBPairData_098,             /**< %2E78..2F0F */
    __SBPairData_098,             /**< %2F10..2FA7 */
    __SBPairData_7B8,             /**< %2FA8..303F */
    __SBPairData_098,             /**< %3040..30D7 */
    __SBPairData_098,             /**< %30D8..316F */
    __SBPairData_098,             /**< %3170..3207 */
    __SBPairData_098,             /**< %3208..329F */
    __SBPairData_098,             /**< %32A0..3337 */
    __SBPairData_098,             /**< %3338..33CF */
    __SBPairData_098,             /**< %33D0..3467 */
    __SBPairData_098,             /**< %3468..34FF */
    __SBPairData_098,             /**< %3500..3597 */
    __SBPairData_098,             /**< %3598..362F */
    __SBPairData_098,             /**< %3630..36C7 */
    __SBPairData_098,             /**< %36C8..375F */
    __SBPairData_098,             /**< %3760..37F7 */
    __SBPairData_098,             /**< %37F8..388F */
    __SBPairData_098,             /**< %3890..3927 */
    __SBPairData_098,             /**< %3928..39BF */
    __SBPairData_098,             /**< %39C0..3A57 */
    __SBPairData_098,             /**< %3A58..3AEF */
    __SBPairData_098,             /**< %3AF0..3B87 */
    __SBPairData_098,             /**< %3B88..3C1F */
    __SBPairData_098,             /**< %3C20..3CB7 */
    __SBPairData_098,             /**< %3CB8..3D4F */
    __SBPairData_098,             /**< %3D50..3DE7 */
    __SBPairData_098,             /**< %3DE8..3E7F */
    __SBPairData_098,             /**< %3E80..3F17 */
    __SBPairData_098,             /**< %3F18..3FAF */
    __SBPairData_098,             /**< %3FB0..4047 */
    __SBPairData_098,             /**< %4048..40DF */
    __SBPairData_098,             /**< %40E0..4177 */
    __SBPairData_098,             /**< %4178..420F */
    __SBPairData_098,             /**< %4210..42A7 */
    __SBPairData_098,             /**< %42A8..433F */
    __SBPairData_098,             /**< %4340..43D7 */
    __SBPairData_098,             /**< %43D8..446F */
    __SBPairData_098,             /**< %4470..4507 */
    __SBPairData_098,             /**< %4508..459F */
    __SBPairData_098,             /**< %45A0..4637 */
    __SBPairData_098,             /**< %4638..46CF */
    __SBPairData_098,             /**< %46D0..4767 */
    __SBPairData_098,             /**< %4768..47FF */
    __SBPairData_098,             /**< %4800..4897 */
    __SBPairData_098,             /**< %4898..492F */
    __SBPairData_098,             /**< %4930..49C7 */
    __SBPairData_098,             /**< %49C8..4A5F */
    __SBPairData_098,             /**< %4A60..4AF7 */
    __SBPairData_098,             /**< %4AF8..4B8F */
    __SBPairData_098,             /**< %4B90..4C27 */
    __SBPairData_098,             /**< %4C28..4CBF */
    __SBPairData_098,             /**< %4CC0..4D57 */
    __SBPairData_098,             /**< %4D58..4DEF */
    __SBPairData_098,             /**< %4DF0..4E87 */
    __SBPairData_098,             /**< %4E88..4F1F */
    __SBPairData_098,             /**< %4F20..4FB7 */
    __SBPairData_098,             /**< %4FB8..504F */
    __SBPairData_098,             /**< %5050..50E7 */
    __SBPairData_098,             /**< %50E8..517F */
    __SBPairData_098,             /**< %5180..5217 */
    __SBPairData_098,             /**< %5218..52AF */
    __SBPairData_098,             /**< %52B0..5347 */
    __SBPairData_098,             /**< %5348..53DF */
    __SBPairData_098,             /**< %53E0..5477 */
    __SBPairData_098,             /**< %5478..550F */
    __SBPairData_098,             /**< %5510..55A7 */
    __SBPairData_098,             /**< %55A8..563F */
    __SBPairData_098,             /**< %5640..56D7 */
    __SBPairData_098,             /**< %56D8..576F */
    __SBPairData_098,             /**< %5770..5807 */
    __SBPairData_098,             /**< %5808..589F */
    __SBPairData_098,             /**< %58A0..5937 */
    __SBPairData_098,             /**< %5938..59CF */
    __SBPairData_098,             /**< %59D0..5A67 */
    __SBPairData_098,             /**< %5A68..5AFF */
    __SBPairData_098,             /**< %5B00..5B97 */
    __SBPairData_098,             /**< %5B98..5C2F */
    __SBPairData_098,             /**< %5C30..5CC7 */
    __SBPairData_098,             /**< %5CC8..5D5F */
    __SBPairData_098,             /**< %5D60..5DF7 */
    __SBPairData_098,             /**< %5DF8..5E8F */
    __SBPairData_098,             /**< %5E90..5F27 */
    __SBPairData_098,             /**< %5F28..5FBF */
    __SBPairData_098,             /**< %5FC0..6057 */
    __SBPairData_098,             /**< %6058..60EF */
    __SBPairData_098,             /**< %60F0..6187 */
    __SBPairData_098,             /**< %6188..621F */
    __SBPairData_098,             /**< %6220..62B7 */
    __SBPairData_098,             /**< %62B8..634F */
    __SBPairData_098,             /**< %6350..63E7 */
    __SBPairData_098,             /**< %63E8..647F */
    __SBPairData_098,             /**< %6480..6517 */
    __SBPairData_098,             /**< %6518..65AF */
    __SBPairData_098,             /**< %65B0..6647 */
    __SBPairData_098,             /**< %6648..66DF */
    __SBPairData_098,             /**< %66E0..6777 */
    __SBPairData_098,             /**< %6778..680F */
    __SBPairData_098,             /**< %6810..68A7 */
    __SBPairData_098,             /**< %68A8..693F */
    __SBPairData_098,             /**< %6940..69D7 */
    __SBPairData_098,             /**< %69D8..6A6F */
    __SBPairData_098,             /**< %6A70..6B07 */
    __SBPairData_098,             /**< %6B08..6B9F */
    __SBPairData_098,             /**< %6BA0..6C37 */
    __SBPairData_098,             /**< %6C38..6CCF */
    __SBPairData_098,             /**< %6CD0..6D67 */
    __SBPairData_098,             /**< %6D68..6DFF */
    __SBPairData_098,             /**< %6E00..6E97 */
    __SBPairData_098,             /**< %6E98..6F2F */
    __SBPairData_098,             /**< %6F30..6FC7 */
    __SBPairData_098,             /**< %6FC8..705F */
    __SBPairData_098,             /**< %7060..70F7 */
    __SBPairData_098,             /**< %70F8..718F */
    __SBPairData_098,             /**< %7190..7227 */
    __SBPairData_098,             /**< %7228..72BF */
    __SBPairData_098,             /**< %72C0..7357 */
    __SBPairData_098,             /**< %7358..73EF */
    __SBPairData_098,             /**< %73F0..7487 */
    __SBPairData_098,             /**< %7488..751F */
    __SBPairData_098,             /**< %7520..75B7 */
    __SBPairData_098,             /**< %75B8..764F */
    __SBPairData_098,             /**< %7650..76E7 */
    __SBPairData_098,             /**< %76E8..777F */
    __SBPairData_098,             /**< %7780..7817 */
    __SBPairData_098,             /**< %7818..78AF */
    __SBPairData_098,             /**< %78B0..7947 */
    __SBPairData_098,             /**< %7948..79DF */
    __SBPairData_098,             /**< %79E0..7A77 */
    __SBPairData_098,             /**< %7A78..7B0F */
    __SBPairData_098,             /**< %7B10..7BA7 */
    __SBPairData_098,             /**< %7BA8..7C3F */
    __SBPairData_098,             /**< %7C40..7CD7 */
    __SBPairData_098,             /**< %7CD8..7D6F */
    __SBPairData_098,             /**< %7D70..7E07 */
    __SBPairData_098,             /**< %7E08..7E9F */
    __SBPairData_098,             /**< %7EA0..7F37 */
    __SBPairData_098,             /**< %7F38..7FCF */
    __SBPairData_098,             /**< %7FD0..8067 */
    __SBPairData_098,             /**< %8068..80FF */
    __SBPairData_098,             /**< %8100..8197 */
    __SBPairData_098,             /**< %8198..822F */
    __SBPairData_098,             /**< %8230..82C7 */
    __SBPairData_098,             /**< %82C8..835F */
    __SBPairData_098,             /**< %8360..83F7 */
    __SBPairData_098,             /**< %83F8..848F */
    __SBPairData_098,             /**< %8490..8527 */
    __SBPairData_098,             /**< %8528..85BF */
    __SBPairData_098,             /**< %85C0..8657 */
    __SBPairData_098,             /**< %8658..86EF */
    __SBPairData_098,             /**< %86F0..8787 */
    __SBPairData_098,             /**< %8788..881F */
    __SBPairData_098,             /**< %8820..88B7 */
    __SBPairData_098,             /**< %88B8..894F */
    __SBPairData_098,             /**< %8950..89E7 */
    __SBPairData_098,             /**< %89E8..8A7F */
    __SBPairData_098,             /**< %8A80..8B17 */
    __SBPairData_098,             /**< %8B18..8BAF */
    __SBPairData_098,             /**< %8BB0..8C47 */
    __SBPairData_098,             /**< %8C48..8CDF */
    __SBPairData_098,             /**< %8CE0..8D77 */
    __SBPairData_098,             /**< %8D78..8E0F */
    __SBPairData_098,             /**< %8E10..8EA7 */
    __SBPairData_098,             /**< %8EA8..8F3F */
    __SBPairData_098,             /**< %8F40..8FD7 */
    __SBPairData_098,             /**< %8FD8..906F */
    __SBPairData_098,             /**< %9070..9107 */
    __SBPairData_098,             /**< %9108..919F */
    __SBPairData_098,             /**< %91A0..9237 */
    __SBPairData_098,             /**< %9238..92CF */
    __SBPairData_098,             /**< %92D0..9367 */
    __SBPairData_098,             /**< %9368..93FF */
    __SBPairData_098,             /**< %9400..9497 */
    __SBPairData_098,             /**< %9498..952F */
    __SBPairData_098,             /**< %9530..95C7 */
    __SBPairData_098,             /**< %95C8..965F */
    __SBPairData_098,             /**< %9660..96F7 */
    __SBPairData_098,             /**< %96F8..978F */
    __SBPairData_098,             /**< %9790..9827 */
    __SBPairData_098,             /**< %9828..98BF */
    __SBPairData_098,             /**< %98C0..9957 */
    __SBPairData_098,             /**< %9958..99EF */
    __SBPairData_098,             /**< %99F0..9A87 */
    __SBPairData_098,             /**< %9A88..9B1F */
    __SBPairData_098,             /**< %9B20..9BB7 */
    __SBPairData_098,             /**< %9BB8..9C4F */
    __SBPairData_098,             /**< %9C50..9CE7 */
    __SBPairData_098,             /**< %9CE8..9D7F */
    __SBPairData_098,             /**< %9D80..9E17 */
    __SBPairData_098,             /**< %9E18..9EAF */
    __SBPairData_098,             /**< %9EB0..9F47 */
    __SBPairData_098,             /**< %9F48..9FDF */
    __SBPairData_098,             /**< %9FE0..A077 */
    __SBPairData_098,             /**< %A078..A10F */
    __SBPairData_098,             /**< %A110..A1A7 */
    __SBPairData_098,             /**< %A1A8..A23F */
    __SBPairData_098,             /**< %A240..A2D7 */
    __SBPairData_098,             /**< %A2D8..A36F */
    __SBPairData_098,             /**< %A370..A407 */
    __SBPairData_098,             /**< %A408..A49F */
    __SBPairData_098,             /**< %A4A0..A537 */
    __SBPairData_098,             /**< %A538..A5CF */
    __SBPairData_098,             /**< %A5D0..A667 */
    __SBPairData_098,             /**< %A668..A6FF */
    __SBPairData_098,             /**< %A700..A797 */
    __SBPairData_098,             /**< %A798..A82F */
    __SBPairData_098,             /**< %A830..A8C7 */
    __SBPairData_098,             /**< %A8C8..A95F */
    __SBPairData_098,             /**< %A960..A9F7 */
    __SBPairData_098,             /**< %A9F8..AA8F */
    __SBPairData_098,             /**< %AA90..AB27 */
    __SBPairData_098,             /**< %AB28..ABBF */
    __SBPairData_098,             /**< %ABC0..AC57 */
    __SBPairData_098,             /**< %AC58..ACEF */
    __SBPairData_098,             /**< %ACF0..AD87 */
    __SBPairData_098,             /**< %AD88..AE1F */
    __SBPairData_098,             /**< %AE20..AEB7 */
    __SBPairData_098,             /**< %AEB8..AF4F */
    __SBPairData_098,             /**< %AF50..AFE7 */
    __SBPairData_098,             /**< %AFE8..B07F */
    __SBPairData_098,             /**< %B080..B117 */
    __SBPairData_098,             /**< %B118..B1AF */
    __SBPairData_098,             /**< %B1B0..B247 */
    __SBPairData_098,             /**< %B248..B2DF */
    __SBPairData_098,             /**< %B2E0..B377 */
    __SBPairData_098,             /**< %B378..B40F */
    __SBPairData_098,             /**< %B410..B4A7 */
    __SBPairData_098,             /**< %B4A8..B53F */
    __SBPairData_098,             /**< %B540..B5D7 */
    __SBPairData_098,             /**< %B5D8..B66F */
    __SBPairData_098,             /**< %B670..B707 */
    __SBPairData_098,             /**< %B708..B79F */
    __SBPairData_098,             /**< %B7A0..B837 */
    __SBPairData_098,             /**< %B838..B8CF */
    __SBPairData_098,             /**< %B8D0..B967 */
    __SBPairData_098,             /**< %B968..B9FF */
    __SBPairData_098,             /**< %BA00..BA97 */
    __SBPairData_098,             /**< %BA98..BB2F */
    __SBPairData_098,             /**< %BB30..BBC7 */
    __SBPairData_098,             /**< %BBC8..BC5F */
    __SBPairData_098,             /**< %BC60..BCF7 */
    __SBPairData_098,             /**< %BCF8..BD8F */
    __SBPairData_098,             /**< %BD90..BE27 */
    __SBPairData_098,             /**< %BE28..BEBF */
    __SBPairData_098,             /**< %BEC0..BF57 */
    __SBPairData_098,             /**< %BF58..BFEF */
    __SBPairData_098,             /**< %BFF0..C087 */
    __SBPairData_098,             /**< %C088..C11F */
    __SBPairData_098,             /**< %C120..C1B7 */
    __SBPairData_098,             /**< %C1B8..C24F */
    __SBPairData_098,             /**< %C250..C2E7 */
    __SBPairData_098,             /**< %C2E8..C37F */
    __SBPairData_098,             /**< %C380..C417 */
    __SBPairData_098,             /**< %C418..C4AF */
    __SBPairData_098,             /**< %C4B0..C547 */
    __SBPairData_098,             /**< %C548..C5DF */
    __SBPairData_098,             /**< %C5E0..C677 */
    __SBPairData_098,             /**< %C678..C70F */
    __SBPairData_098,             /**< %C710..C7A7 */
    __SBPairData_098,             /**< %C7A8..C83F */
    __SBPairData_098,             /**< %C840..C8D7 */
    __SBPairData_098,             /**< %C8D8..C96F */
    __SBPairData_098,             /**< %C970..CA07 */
    __SBPairData_098,             /**< %CA08..CA9F */
    __SBPairData_098,             /**< %CAA0..CB37 */
    __SBPairData_098,             /**< %CB38..CBCF */
    __SBPairData_098,             /**< %CBD0..CC67 */
    __SBPairData_098,             /**< %CC68..CCFF */
    __SBPairData_098,             /**< %CD00..CD97 */
    __SBPairData_098,             /**< %CD98..CE2F */
    __SBPairData_098,             /**< %CE30..CEC7 */
    __SBPairData_098,             /**< %CEC8..CF5F */
    __SBPairData_098,             /**< %CF60..CFF7 */
    __SBPairData_098,             /**< %CFF8..D08F */
    __SBPairData_098,             /**< %D090..D127 */
    __SBPairData_098,             /**< %D128..D1BF */
    __SBPairData_098,             /**< %D1C0..D257 */
    __SBPairData_098,             /**< %D258..D2EF */
    __SBPairData_098,             /**< %D2F0..D387 */
    __SBPairData_098,             /**< %D388..D41F */
    __SBPairData_098,             /**< %D420..D4B7 */
    __SBPairData_098,             /**< %D4B8..D54F */
    __SBPairData_098,             /**< %D550..D5E7 */
    __SBPairData_098,             /**< %D5E8..D67F */
    __SBPairData_098,             /**< %D680..D717 */
    __SBPairData_098,             /**< %D718..D7AF */
    __SBPairData_098,             /**< %D7B0..D847 */
    __SBPairData_098,             /**< %D848..D8DF */
    __SBPairData_098,             /**< %D8E0..D977 */
    __SBPairData_098,             /**< %D978..DA0F */
    __SBPairData_098,             /**< %DA10..DAA7 */
    __SBPairData_098,             /**< %DAA8..DB3F */
    __SBPairData_098,             /**< %DB40..DBD7 */
    __SBPairData_098,             /**< %DBD8..DC6F */
    __SBPairData_098,             /**< %DC70..DD07 */
    __SBPairData_098,             /**< %DD08..DD9F */
    __SBPairData_098,             /**< %DDA0..DE37 */
    __SBPairData_098,             /**< %DE38..DECF */
    __SBPairData_098,             /**< %DED0..DF67 */
    __SBPairData_098,             /**< %DF68..DFFF */
    __SBPairData_098,             /**< %E000..E097 */
    __SBPairData_098,             /**< %E098..E12F */
    __SBPairData_098,             /**< %E130..E1C7 */
    __SBPairData_098,             /**< %E1C8..E25F */
    __SBPairData_098,             /**< %E260..E2F7 */
    __SBPairData_098,             /**< %E2F8..E38F */
    __SBPairData_098,             /**< %E390..E427 */
    __SBPairData_098,             /**< %E428..E4BF */
    __SBPairData_098,             /**< %E4C0..E557 */
    __SBPairData_098,             /**< %E558..E5EF */
    __SBPairData_098,             /**< %E5F0..E687 */
    __SBPairData_098,             /**< %E688..E71F */
    __SBPairData_098,             /**< %E720..E7B7 */
    __SBPairData_098,             /**< %E7B8..E84F */
    __SBPairData_098,             /**< %E850..E8E7 */
    __SBPairData_098,             /**< %E8E8..E97F */
    __SBPairData_098,             /**< %E980..EA17 */
    __SBPairData_098,             /**< %EA18..EAAF */
    __SBPairData_098,             /**< %EAB0..EB47 */
    __SBPairData_098,             /**< %EB48..EBDF */
    __SBPairData_098,             /**< %EBE0..EC77 */
    __SBPairData_098,             /**< %EC78..ED0F */
    __SBPairData_098,             /**< %ED10..EDA7 */
    __SBPairData_098,             /**< %EDA8..EE3F */
    __SBPairData_098,             /**< %EE40..EED7 */
    __SBPairData_098,             /**< %EED8..EF6F */
    __SBPairData_098,             /**< %EF70..F007 */
    __SBPairData_098,             /**< %F008..F09F */
    __SBPairData_098,             /**< %F0A0..F137 */
    __SBPairData_098,             /**< %F138..F1CF */
    __SBPairData_098,             /**< %F1D0..F267 */
    __SBPairData_098,             /**< %F268..F2FF */
    __SBPairData_098,             /**< %F300..F397 */
    __SBPairData_098,             /**< %F398..F42F */
    __SBPairData_098,             /**< %F430..F4C7 */
    __SBPairData_098,             /**< %F4C8..F55F */
    __SBPairData_098,             /**< %F560..F5F7 */
    __SBPairData_098,             /**< %F5F8..F68F */
    __SBPairData_098,             /**< %F690..F727 */
    __SBPairData_098,             /**< %F728..F7BF */
    __SBPairData_098,             /**< %F7C0..F857 */
    __SBPairData_098,             /**< %F858..F8EF */
    __SBPairData_098,             /**< %F8F0..F987 */
    __SBPairData_098,             /**< %F988..FA1F */
    __SBPairData_098,             /**< %FA20..FAB7 */
    __SBPairData_098,             /**< %FAB8..FB4F */
    __SBPairData_098,             /**< %FB50..FBE7 */
    __SBPairData_098,             /**< %FBE8..FC7F */
    __SBPairData_098,             /**< %FC80..FD17 */
    __SBPairData_098,             /**< %FD18..FDAF */
    __SBPairData_098,             /**< %FDB0..FE47 */
    __SBPairData_850,             /**< %FE48..FEDF */
    __SBPairData_8E8              /**< %FEE0..FF63 */
};

SB_INTERNAL SBUnichar _SBPairingDetermineMirror(SBUnichar unicode) {
    SBUInt32 trim = unicode - 40;

    if (trim <= 0xFF3B) {
        SBInt16 diff = __SBPairDifferences[
                        __SBPairData[
                         __SBPairIndexes[
                              trim / 152
                         ] + (trim % 152)
                        ] & _SB_BRACKET_TYPE__INVERSE_MASK
                       ];

        if (diff != 0) {
            return (unicode + diff);
        }
    }
    
    return 0;
}

SB_INTERNAL SBUnichar _SBPairingDetermineBracketPair(SBUnichar unicode, _SBBracketType *type) {
    SBUInt32 trim = unicode - 40;

    if (trim <= 0xFF3B) {
        SBUInt8 data = __SBPairData[
                        __SBPairIndexes[
                             trim / 152
                        ] + (trim % 152)
                       ];
        *type = (data & _SB_BRACKET_TYPE__PRIMARY_MASK);

        if (*type != 0) {
            SBInt16 diff = __SBPairDifferences[
                            data & _SB_BRACKET_TYPE__INVERSE_MASK
                           ];
            return (unicode + diff);
        }
    } else {
        *type = _SB_BRACKET_TYPE__NONE;
    }

    return 0;
}
