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

const SBInt16 _SBPairDifferences[] = {
    0,     1,     -1,    2,     -2,    16,    -16,   3,     -3,    2016,  138,
    1824,  2104,  2108,  2106,  -138,  8,     7,     -8,    -7,    -1824, -2016,
    -2104, -2106, -2108
};

const SBUInt8 _SBPairData[] = {
#define _SBPairData_000 0x000
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
#define _SBPairData_098 0x098
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
#define _SBPairData_130 0x130
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
#define _SBPairData_1C8 0x1C8
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
#define _SBPairData_260 0x260
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
#define _SBPairData_2F8 0x2F8
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
#define _SBPairData_390 0x390
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
#define _SBPairData_428 0x428
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
#define _SBPairData_4C0 0x4C0
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
#define _SBPairData_558 0x558
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
#define _SBPairData_5F0 0x5F0
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
#define _SBPairData_688 0x688
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
#define _SBPairData_720 0x720
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
#define _SBPairData_7B8 0x7B8
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
#define _SBPairData_850 0x850
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
#define _SBPairData_8E8 0x8E8
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

const SBUInt16 _SBPairIndexes[] = {
    _SBPairData_000,             /**< %0028..00BF */
    _SBPairData_098,             /**< %00C0..0157 */
    _SBPairData_098,             /**< %0158..01EF */
    _SBPairData_098,             /**< %01F0..0287 */
    _SBPairData_098,             /**< %0288..031F */
    _SBPairData_098,             /**< %0320..03B7 */
    _SBPairData_098,             /**< %03B8..044F */
    _SBPairData_098,             /**< %0450..04E7 */
    _SBPairData_098,             /**< %04E8..057F */
    _SBPairData_098,             /**< %0580..0617 */
    _SBPairData_098,             /**< %0618..06AF */
    _SBPairData_098,             /**< %06B0..0747 */
    _SBPairData_098,             /**< %0748..07DF */
    _SBPairData_098,             /**< %07E0..0877 */
    _SBPairData_098,             /**< %0878..090F */
    _SBPairData_098,             /**< %0910..09A7 */
    _SBPairData_098,             /**< %09A8..0A3F */
    _SBPairData_098,             /**< %0A40..0AD7 */
    _SBPairData_098,             /**< %0AD8..0B6F */
    _SBPairData_098,             /**< %0B70..0C07 */
    _SBPairData_098,             /**< %0C08..0C9F */
    _SBPairData_098,             /**< %0CA0..0D37 */
    _SBPairData_098,             /**< %0D38..0DCF */
    _SBPairData_098,             /**< %0DD0..0E67 */
    _SBPairData_098,             /**< %0E68..0EFF */
    _SBPairData_130,             /**< %0F00..0F97 */
    _SBPairData_098,             /**< %0F98..102F */
    _SBPairData_098,             /**< %1030..10C7 */
    _SBPairData_098,             /**< %10C8..115F */
    _SBPairData_098,             /**< %1160..11F7 */
    _SBPairData_098,             /**< %11F8..128F */
    _SBPairData_098,             /**< %1290..1327 */
    _SBPairData_098,             /**< %1328..13BF */
    _SBPairData_098,             /**< %13C0..1457 */
    _SBPairData_098,             /**< %1458..14EF */
    _SBPairData_098,             /**< %14F0..1587 */
    _SBPairData_098,             /**< %1588..161F */
    _SBPairData_1C8,             /**< %1620..16B7 */
    _SBPairData_098,             /**< %16B8..174F */
    _SBPairData_098,             /**< %1750..17E7 */
    _SBPairData_098,             /**< %17E8..187F */
    _SBPairData_098,             /**< %1880..1917 */
    _SBPairData_098,             /**< %1918..19AF */
    _SBPairData_098,             /**< %19B0..1A47 */
    _SBPairData_098,             /**< %1A48..1ADF */
    _SBPairData_098,             /**< %1AE0..1B77 */
    _SBPairData_098,             /**< %1B78..1C0F */
    _SBPairData_098,             /**< %1C10..1CA7 */
    _SBPairData_098,             /**< %1CA8..1D3F */
    _SBPairData_098,             /**< %1D40..1DD7 */
    _SBPairData_098,             /**< %1DD8..1E6F */
    _SBPairData_098,             /**< %1E70..1F07 */
    _SBPairData_098,             /**< %1F08..1F9F */
    _SBPairData_098,             /**< %1FA0..2037 */
    _SBPairData_260,             /**< %2038..20CF */
    _SBPairData_098,             /**< %20D0..2167 */
    _SBPairData_098,             /**< %2168..21FF */
    _SBPairData_2F8,             /**< %2200..2297 */
    _SBPairData_390,             /**< %2298..232F */
    _SBPairData_098,             /**< %2330..23C7 */
    _SBPairData_098,             /**< %23C8..245F */
    _SBPairData_098,             /**< %2460..24F7 */
    _SBPairData_098,             /**< %24F8..258F */
    _SBPairData_098,             /**< %2590..2627 */
    _SBPairData_098,             /**< %2628..26BF */
    _SBPairData_098,             /**< %26C0..2757 */
    _SBPairData_428,             /**< %2758..27EF */
    _SBPairData_098,             /**< %27F0..2887 */
    _SBPairData_098,             /**< %2888..291F */
    _SBPairData_4C0,             /**< %2920..29B7 */
    _SBPairData_558,             /**< %29B8..2A4F */
    _SBPairData_5F0,             /**< %2A50..2AE7 */
    _SBPairData_688,             /**< %2AE8..2B7F */
    _SBPairData_098,             /**< %2B80..2C17 */
    _SBPairData_098,             /**< %2C18..2CAF */
    _SBPairData_098,             /**< %2CB0..2D47 */
    _SBPairData_098,             /**< %2D48..2DDF */
    _SBPairData_720,             /**< %2DE0..2E77 */
    _SBPairData_098,             /**< %2E78..2F0F */
    _SBPairData_098,             /**< %2F10..2FA7 */
    _SBPairData_7B8,             /**< %2FA8..303F */
    _SBPairData_098,             /**< %3040..30D7 */
    _SBPairData_098,             /**< %30D8..316F */
    _SBPairData_098,             /**< %3170..3207 */
    _SBPairData_098,             /**< %3208..329F */
    _SBPairData_098,             /**< %32A0..3337 */
    _SBPairData_098,             /**< %3338..33CF */
    _SBPairData_098,             /**< %33D0..3467 */
    _SBPairData_098,             /**< %3468..34FF */
    _SBPairData_098,             /**< %3500..3597 */
    _SBPairData_098,             /**< %3598..362F */
    _SBPairData_098,             /**< %3630..36C7 */
    _SBPairData_098,             /**< %36C8..375F */
    _SBPairData_098,             /**< %3760..37F7 */
    _SBPairData_098,             /**< %37F8..388F */
    _SBPairData_098,             /**< %3890..3927 */
    _SBPairData_098,             /**< %3928..39BF */
    _SBPairData_098,             /**< %39C0..3A57 */
    _SBPairData_098,             /**< %3A58..3AEF */
    _SBPairData_098,             /**< %3AF0..3B87 */
    _SBPairData_098,             /**< %3B88..3C1F */
    _SBPairData_098,             /**< %3C20..3CB7 */
    _SBPairData_098,             /**< %3CB8..3D4F */
    _SBPairData_098,             /**< %3D50..3DE7 */
    _SBPairData_098,             /**< %3DE8..3E7F */
    _SBPairData_098,             /**< %3E80..3F17 */
    _SBPairData_098,             /**< %3F18..3FAF */
    _SBPairData_098,             /**< %3FB0..4047 */
    _SBPairData_098,             /**< %4048..40DF */
    _SBPairData_098,             /**< %40E0..4177 */
    _SBPairData_098,             /**< %4178..420F */
    _SBPairData_098,             /**< %4210..42A7 */
    _SBPairData_098,             /**< %42A8..433F */
    _SBPairData_098,             /**< %4340..43D7 */
    _SBPairData_098,             /**< %43D8..446F */
    _SBPairData_098,             /**< %4470..4507 */
    _SBPairData_098,             /**< %4508..459F */
    _SBPairData_098,             /**< %45A0..4637 */
    _SBPairData_098,             /**< %4638..46CF */
    _SBPairData_098,             /**< %46D0..4767 */
    _SBPairData_098,             /**< %4768..47FF */
    _SBPairData_098,             /**< %4800..4897 */
    _SBPairData_098,             /**< %4898..492F */
    _SBPairData_098,             /**< %4930..49C7 */
    _SBPairData_098,             /**< %49C8..4A5F */
    _SBPairData_098,             /**< %4A60..4AF7 */
    _SBPairData_098,             /**< %4AF8..4B8F */
    _SBPairData_098,             /**< %4B90..4C27 */
    _SBPairData_098,             /**< %4C28..4CBF */
    _SBPairData_098,             /**< %4CC0..4D57 */
    _SBPairData_098,             /**< %4D58..4DEF */
    _SBPairData_098,             /**< %4DF0..4E87 */
    _SBPairData_098,             /**< %4E88..4F1F */
    _SBPairData_098,             /**< %4F20..4FB7 */
    _SBPairData_098,             /**< %4FB8..504F */
    _SBPairData_098,             /**< %5050..50E7 */
    _SBPairData_098,             /**< %50E8..517F */
    _SBPairData_098,             /**< %5180..5217 */
    _SBPairData_098,             /**< %5218..52AF */
    _SBPairData_098,             /**< %52B0..5347 */
    _SBPairData_098,             /**< %5348..53DF */
    _SBPairData_098,             /**< %53E0..5477 */
    _SBPairData_098,             /**< %5478..550F */
    _SBPairData_098,             /**< %5510..55A7 */
    _SBPairData_098,             /**< %55A8..563F */
    _SBPairData_098,             /**< %5640..56D7 */
    _SBPairData_098,             /**< %56D8..576F */
    _SBPairData_098,             /**< %5770..5807 */
    _SBPairData_098,             /**< %5808..589F */
    _SBPairData_098,             /**< %58A0..5937 */
    _SBPairData_098,             /**< %5938..59CF */
    _SBPairData_098,             /**< %59D0..5A67 */
    _SBPairData_098,             /**< %5A68..5AFF */
    _SBPairData_098,             /**< %5B00..5B97 */
    _SBPairData_098,             /**< %5B98..5C2F */
    _SBPairData_098,             /**< %5C30..5CC7 */
    _SBPairData_098,             /**< %5CC8..5D5F */
    _SBPairData_098,             /**< %5D60..5DF7 */
    _SBPairData_098,             /**< %5DF8..5E8F */
    _SBPairData_098,             /**< %5E90..5F27 */
    _SBPairData_098,             /**< %5F28..5FBF */
    _SBPairData_098,             /**< %5FC0..6057 */
    _SBPairData_098,             /**< %6058..60EF */
    _SBPairData_098,             /**< %60F0..6187 */
    _SBPairData_098,             /**< %6188..621F */
    _SBPairData_098,             /**< %6220..62B7 */
    _SBPairData_098,             /**< %62B8..634F */
    _SBPairData_098,             /**< %6350..63E7 */
    _SBPairData_098,             /**< %63E8..647F */
    _SBPairData_098,             /**< %6480..6517 */
    _SBPairData_098,             /**< %6518..65AF */
    _SBPairData_098,             /**< %65B0..6647 */
    _SBPairData_098,             /**< %6648..66DF */
    _SBPairData_098,             /**< %66E0..6777 */
    _SBPairData_098,             /**< %6778..680F */
    _SBPairData_098,             /**< %6810..68A7 */
    _SBPairData_098,             /**< %68A8..693F */
    _SBPairData_098,             /**< %6940..69D7 */
    _SBPairData_098,             /**< %69D8..6A6F */
    _SBPairData_098,             /**< %6A70..6B07 */
    _SBPairData_098,             /**< %6B08..6B9F */
    _SBPairData_098,             /**< %6BA0..6C37 */
    _SBPairData_098,             /**< %6C38..6CCF */
    _SBPairData_098,             /**< %6CD0..6D67 */
    _SBPairData_098,             /**< %6D68..6DFF */
    _SBPairData_098,             /**< %6E00..6E97 */
    _SBPairData_098,             /**< %6E98..6F2F */
    _SBPairData_098,             /**< %6F30..6FC7 */
    _SBPairData_098,             /**< %6FC8..705F */
    _SBPairData_098,             /**< %7060..70F7 */
    _SBPairData_098,             /**< %70F8..718F */
    _SBPairData_098,             /**< %7190..7227 */
    _SBPairData_098,             /**< %7228..72BF */
    _SBPairData_098,             /**< %72C0..7357 */
    _SBPairData_098,             /**< %7358..73EF */
    _SBPairData_098,             /**< %73F0..7487 */
    _SBPairData_098,             /**< %7488..751F */
    _SBPairData_098,             /**< %7520..75B7 */
    _SBPairData_098,             /**< %75B8..764F */
    _SBPairData_098,             /**< %7650..76E7 */
    _SBPairData_098,             /**< %76E8..777F */
    _SBPairData_098,             /**< %7780..7817 */
    _SBPairData_098,             /**< %7818..78AF */
    _SBPairData_098,             /**< %78B0..7947 */
    _SBPairData_098,             /**< %7948..79DF */
    _SBPairData_098,             /**< %79E0..7A77 */
    _SBPairData_098,             /**< %7A78..7B0F */
    _SBPairData_098,             /**< %7B10..7BA7 */
    _SBPairData_098,             /**< %7BA8..7C3F */
    _SBPairData_098,             /**< %7C40..7CD7 */
    _SBPairData_098,             /**< %7CD8..7D6F */
    _SBPairData_098,             /**< %7D70..7E07 */
    _SBPairData_098,             /**< %7E08..7E9F */
    _SBPairData_098,             /**< %7EA0..7F37 */
    _SBPairData_098,             /**< %7F38..7FCF */
    _SBPairData_098,             /**< %7FD0..8067 */
    _SBPairData_098,             /**< %8068..80FF */
    _SBPairData_098,             /**< %8100..8197 */
    _SBPairData_098,             /**< %8198..822F */
    _SBPairData_098,             /**< %8230..82C7 */
    _SBPairData_098,             /**< %82C8..835F */
    _SBPairData_098,             /**< %8360..83F7 */
    _SBPairData_098,             /**< %83F8..848F */
    _SBPairData_098,             /**< %8490..8527 */
    _SBPairData_098,             /**< %8528..85BF */
    _SBPairData_098,             /**< %85C0..8657 */
    _SBPairData_098,             /**< %8658..86EF */
    _SBPairData_098,             /**< %86F0..8787 */
    _SBPairData_098,             /**< %8788..881F */
    _SBPairData_098,             /**< %8820..88B7 */
    _SBPairData_098,             /**< %88B8..894F */
    _SBPairData_098,             /**< %8950..89E7 */
    _SBPairData_098,             /**< %89E8..8A7F */
    _SBPairData_098,             /**< %8A80..8B17 */
    _SBPairData_098,             /**< %8B18..8BAF */
    _SBPairData_098,             /**< %8BB0..8C47 */
    _SBPairData_098,             /**< %8C48..8CDF */
    _SBPairData_098,             /**< %8CE0..8D77 */
    _SBPairData_098,             /**< %8D78..8E0F */
    _SBPairData_098,             /**< %8E10..8EA7 */
    _SBPairData_098,             /**< %8EA8..8F3F */
    _SBPairData_098,             /**< %8F40..8FD7 */
    _SBPairData_098,             /**< %8FD8..906F */
    _SBPairData_098,             /**< %9070..9107 */
    _SBPairData_098,             /**< %9108..919F */
    _SBPairData_098,             /**< %91A0..9237 */
    _SBPairData_098,             /**< %9238..92CF */
    _SBPairData_098,             /**< %92D0..9367 */
    _SBPairData_098,             /**< %9368..93FF */
    _SBPairData_098,             /**< %9400..9497 */
    _SBPairData_098,             /**< %9498..952F */
    _SBPairData_098,             /**< %9530..95C7 */
    _SBPairData_098,             /**< %95C8..965F */
    _SBPairData_098,             /**< %9660..96F7 */
    _SBPairData_098,             /**< %96F8..978F */
    _SBPairData_098,             /**< %9790..9827 */
    _SBPairData_098,             /**< %9828..98BF */
    _SBPairData_098,             /**< %98C0..9957 */
    _SBPairData_098,             /**< %9958..99EF */
    _SBPairData_098,             /**< %99F0..9A87 */
    _SBPairData_098,             /**< %9A88..9B1F */
    _SBPairData_098,             /**< %9B20..9BB7 */
    _SBPairData_098,             /**< %9BB8..9C4F */
    _SBPairData_098,             /**< %9C50..9CE7 */
    _SBPairData_098,             /**< %9CE8..9D7F */
    _SBPairData_098,             /**< %9D80..9E17 */
    _SBPairData_098,             /**< %9E18..9EAF */
    _SBPairData_098,             /**< %9EB0..9F47 */
    _SBPairData_098,             /**< %9F48..9FDF */
    _SBPairData_098,             /**< %9FE0..A077 */
    _SBPairData_098,             /**< %A078..A10F */
    _SBPairData_098,             /**< %A110..A1A7 */
    _SBPairData_098,             /**< %A1A8..A23F */
    _SBPairData_098,             /**< %A240..A2D7 */
    _SBPairData_098,             /**< %A2D8..A36F */
    _SBPairData_098,             /**< %A370..A407 */
    _SBPairData_098,             /**< %A408..A49F */
    _SBPairData_098,             /**< %A4A0..A537 */
    _SBPairData_098,             /**< %A538..A5CF */
    _SBPairData_098,             /**< %A5D0..A667 */
    _SBPairData_098,             /**< %A668..A6FF */
    _SBPairData_098,             /**< %A700..A797 */
    _SBPairData_098,             /**< %A798..A82F */
    _SBPairData_098,             /**< %A830..A8C7 */
    _SBPairData_098,             /**< %A8C8..A95F */
    _SBPairData_098,             /**< %A960..A9F7 */
    _SBPairData_098,             /**< %A9F8..AA8F */
    _SBPairData_098,             /**< %AA90..AB27 */
    _SBPairData_098,             /**< %AB28..ABBF */
    _SBPairData_098,             /**< %ABC0..AC57 */
    _SBPairData_098,             /**< %AC58..ACEF */
    _SBPairData_098,             /**< %ACF0..AD87 */
    _SBPairData_098,             /**< %AD88..AE1F */
    _SBPairData_098,             /**< %AE20..AEB7 */
    _SBPairData_098,             /**< %AEB8..AF4F */
    _SBPairData_098,             /**< %AF50..AFE7 */
    _SBPairData_098,             /**< %AFE8..B07F */
    _SBPairData_098,             /**< %B080..B117 */
    _SBPairData_098,             /**< %B118..B1AF */
    _SBPairData_098,             /**< %B1B0..B247 */
    _SBPairData_098,             /**< %B248..B2DF */
    _SBPairData_098,             /**< %B2E0..B377 */
    _SBPairData_098,             /**< %B378..B40F */
    _SBPairData_098,             /**< %B410..B4A7 */
    _SBPairData_098,             /**< %B4A8..B53F */
    _SBPairData_098,             /**< %B540..B5D7 */
    _SBPairData_098,             /**< %B5D8..B66F */
    _SBPairData_098,             /**< %B670..B707 */
    _SBPairData_098,             /**< %B708..B79F */
    _SBPairData_098,             /**< %B7A0..B837 */
    _SBPairData_098,             /**< %B838..B8CF */
    _SBPairData_098,             /**< %B8D0..B967 */
    _SBPairData_098,             /**< %B968..B9FF */
    _SBPairData_098,             /**< %BA00..BA97 */
    _SBPairData_098,             /**< %BA98..BB2F */
    _SBPairData_098,             /**< %BB30..BBC7 */
    _SBPairData_098,             /**< %BBC8..BC5F */
    _SBPairData_098,             /**< %BC60..BCF7 */
    _SBPairData_098,             /**< %BCF8..BD8F */
    _SBPairData_098,             /**< %BD90..BE27 */
    _SBPairData_098,             /**< %BE28..BEBF */
    _SBPairData_098,             /**< %BEC0..BF57 */
    _SBPairData_098,             /**< %BF58..BFEF */
    _SBPairData_098,             /**< %BFF0..C087 */
    _SBPairData_098,             /**< %C088..C11F */
    _SBPairData_098,             /**< %C120..C1B7 */
    _SBPairData_098,             /**< %C1B8..C24F */
    _SBPairData_098,             /**< %C250..C2E7 */
    _SBPairData_098,             /**< %C2E8..C37F */
    _SBPairData_098,             /**< %C380..C417 */
    _SBPairData_098,             /**< %C418..C4AF */
    _SBPairData_098,             /**< %C4B0..C547 */
    _SBPairData_098,             /**< %C548..C5DF */
    _SBPairData_098,             /**< %C5E0..C677 */
    _SBPairData_098,             /**< %C678..C70F */
    _SBPairData_098,             /**< %C710..C7A7 */
    _SBPairData_098,             /**< %C7A8..C83F */
    _SBPairData_098,             /**< %C840..C8D7 */
    _SBPairData_098,             /**< %C8D8..C96F */
    _SBPairData_098,             /**< %C970..CA07 */
    _SBPairData_098,             /**< %CA08..CA9F */
    _SBPairData_098,             /**< %CAA0..CB37 */
    _SBPairData_098,             /**< %CB38..CBCF */
    _SBPairData_098,             /**< %CBD0..CC67 */
    _SBPairData_098,             /**< %CC68..CCFF */
    _SBPairData_098,             /**< %CD00..CD97 */
    _SBPairData_098,             /**< %CD98..CE2F */
    _SBPairData_098,             /**< %CE30..CEC7 */
    _SBPairData_098,             /**< %CEC8..CF5F */
    _SBPairData_098,             /**< %CF60..CFF7 */
    _SBPairData_098,             /**< %CFF8..D08F */
    _SBPairData_098,             /**< %D090..D127 */
    _SBPairData_098,             /**< %D128..D1BF */
    _SBPairData_098,             /**< %D1C0..D257 */
    _SBPairData_098,             /**< %D258..D2EF */
    _SBPairData_098,             /**< %D2F0..D387 */
    _SBPairData_098,             /**< %D388..D41F */
    _SBPairData_098,             /**< %D420..D4B7 */
    _SBPairData_098,             /**< %D4B8..D54F */
    _SBPairData_098,             /**< %D550..D5E7 */
    _SBPairData_098,             /**< %D5E8..D67F */
    _SBPairData_098,             /**< %D680..D717 */
    _SBPairData_098,             /**< %D718..D7AF */
    _SBPairData_098,             /**< %D7B0..D847 */
    _SBPairData_098,             /**< %D848..D8DF */
    _SBPairData_098,             /**< %D8E0..D977 */
    _SBPairData_098,             /**< %D978..DA0F */
    _SBPairData_098,             /**< %DA10..DAA7 */
    _SBPairData_098,             /**< %DAA8..DB3F */
    _SBPairData_098,             /**< %DB40..DBD7 */
    _SBPairData_098,             /**< %DBD8..DC6F */
    _SBPairData_098,             /**< %DC70..DD07 */
    _SBPairData_098,             /**< %DD08..DD9F */
    _SBPairData_098,             /**< %DDA0..DE37 */
    _SBPairData_098,             /**< %DE38..DECF */
    _SBPairData_098,             /**< %DED0..DF67 */
    _SBPairData_098,             /**< %DF68..DFFF */
    _SBPairData_098,             /**< %E000..E097 */
    _SBPairData_098,             /**< %E098..E12F */
    _SBPairData_098,             /**< %E130..E1C7 */
    _SBPairData_098,             /**< %E1C8..E25F */
    _SBPairData_098,             /**< %E260..E2F7 */
    _SBPairData_098,             /**< %E2F8..E38F */
    _SBPairData_098,             /**< %E390..E427 */
    _SBPairData_098,             /**< %E428..E4BF */
    _SBPairData_098,             /**< %E4C0..E557 */
    _SBPairData_098,             /**< %E558..E5EF */
    _SBPairData_098,             /**< %E5F0..E687 */
    _SBPairData_098,             /**< %E688..E71F */
    _SBPairData_098,             /**< %E720..E7B7 */
    _SBPairData_098,             /**< %E7B8..E84F */
    _SBPairData_098,             /**< %E850..E8E7 */
    _SBPairData_098,             /**< %E8E8..E97F */
    _SBPairData_098,             /**< %E980..EA17 */
    _SBPairData_098,             /**< %EA18..EAAF */
    _SBPairData_098,             /**< %EAB0..EB47 */
    _SBPairData_098,             /**< %EB48..EBDF */
    _SBPairData_098,             /**< %EBE0..EC77 */
    _SBPairData_098,             /**< %EC78..ED0F */
    _SBPairData_098,             /**< %ED10..EDA7 */
    _SBPairData_098,             /**< %EDA8..EE3F */
    _SBPairData_098,             /**< %EE40..EED7 */
    _SBPairData_098,             /**< %EED8..EF6F */
    _SBPairData_098,             /**< %EF70..F007 */
    _SBPairData_098,             /**< %F008..F09F */
    _SBPairData_098,             /**< %F0A0..F137 */
    _SBPairData_098,             /**< %F138..F1CF */
    _SBPairData_098,             /**< %F1D0..F267 */
    _SBPairData_098,             /**< %F268..F2FF */
    _SBPairData_098,             /**< %F300..F397 */
    _SBPairData_098,             /**< %F398..F42F */
    _SBPairData_098,             /**< %F430..F4C7 */
    _SBPairData_098,             /**< %F4C8..F55F */
    _SBPairData_098,             /**< %F560..F5F7 */
    _SBPairData_098,             /**< %F5F8..F68F */
    _SBPairData_098,             /**< %F690..F727 */
    _SBPairData_098,             /**< %F728..F7BF */
    _SBPairData_098,             /**< %F7C0..F857 */
    _SBPairData_098,             /**< %F858..F8EF */
    _SBPairData_098,             /**< %F8F0..F987 */
    _SBPairData_098,             /**< %F988..FA1F */
    _SBPairData_098,             /**< %FA20..FAB7 */
    _SBPairData_098,             /**< %FAB8..FB4F */
    _SBPairData_098,             /**< %FB50..FBE7 */
    _SBPairData_098,             /**< %FBE8..FC7F */
    _SBPairData_098,             /**< %FC80..FD17 */
    _SBPairData_098,             /**< %FD18..FDAF */
    _SBPairData_098,             /**< %FDB0..FE47 */
    _SBPairData_850,             /**< %FE48..FEDF */
    _SBPairData_8E8              /**< %FEE0..FF63 */
};

SB_INTERNAL SBUnichar SBPairingDetermineMirror(SBUnichar unicode) {
    SBUInt32 trim = unicode - 40;

    if (trim <= 0xFF3B) {
        SBInt16 diff = _SBPairDifferences[
                        _SBPairData[
                         _SBPairIndexes[
                              trim / 152
                         ] + (trim % 152)
                        ] & SB_BRACKET_TYPE__INVERSE_MASK
                       ];

        if (diff != 0) {
            return (unicode + diff);
        }
    }
    
    return 0;
}

SB_INTERNAL SBUnichar SBPairingDetermineBracketPair(SBUnichar unicode, SBBracketType *type) {
    SBUInt32 trim = unicode - 40;

    if (trim <= 0xFF3B) {
        SBUInt8 data = _SBPairData[
                        _SBPairIndexes[
                             trim / 152
                        ] + (trim % 152)
                       ];
        *type = (data & SB_BRACKET_TYPE__PRIMARY_MASK);

        if (*type != 0) {
            SBInt16 diff = _SBPairDifferences[
                            data & SB_BRACKET_TYPE__INVERSE_MASK
                           ];
            return (unicode + diff);
        }
    } else {
        *type = SB_BRACKET_TYPE__NONE;
    }

    return 0;
}
