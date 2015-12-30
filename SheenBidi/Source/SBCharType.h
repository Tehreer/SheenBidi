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

#ifndef _SB_INTERNAL_CHAR_TYPE_H
#define _SB_INTERNAL_CHAR_TYPE_H

#include <SBTypes.h>

#define SB_CHAR_TYPE                            SBUInt8

#define SB_CHAR_TYPE__NIL                       0x00

#define SB_CHAR_TYPE__L                         0x01
#define SB_CHAR_TYPE__R                         0x02
#define SB_CHAR_TYPE__AL                        0x03

#define SB_CHAR_TYPE__STRONG_TOP                SB_CHAR_TYPE__L
#define SB_CHAR_TYPE__STRONG_END                SB_CHAR_TYPE__AL
#define SB_CHAR_TYPE__STRONG_LIMIT              \
(                                               \
   SB_CHAR_TYPE__STRONG_END                     \
 - SB_CHAR_TYPE__STRONG_TOP                     \
)

#define SB_CHAR_TYPE__EN                        0x04
#define SB_CHAR_TYPE__AN                        0x05
#define SB_CHAR_TYPE__ET                        0x06
#define SB_CHAR_TYPE__NSM                       0x07
#define SB_CHAR_TYPE__BN                        0x08
#define SB_CHAR_TYPE__CS                        0x09
#define SB_CHAR_TYPE__ES                        0x0A

#define SB_CHAR_TYPE__NUMBER_TOP                SB_CHAR_TYPE__EN
#define SB_CHAR_TYPE__NUMBER_END                SB_CHAR_TYPE__AN
#define SB_CHAR_TYPE__NUMBER_LIMIT              \
(                                               \
   SB_CHAR_TYPE__NUMBER_END                     \
 - SB_CHAR_TYPE__NUMBER_TOP                     \
)

#define SB_CHAR_TYPE__STRONG_OR_NUMBER_TOP      SB_CHAR_TYPE__STRONG_TOP
#define SB_CHAR_TYPE__STRONG_OR_NUMBER_END      SB_CHAR_TYPE__NUMBER_END
#define SB_CHAR_TYPE__STRONG_OR_NUMBER_LIMIT    \
(                                               \
  SB_CHAR_TYPE__STRONG_OR_NUMBER_END            \
- SB_CHAR_TYPE__STRONG_OR_NUMBER_TOP            \
)

#define SB_CHAR_TYPE__WEAK_TOP                  SB_CHAR_TYPE__EN
#define SB_CHAR_TYPE__WEAK_END                  SB_CHAR_TYPE__ES
#define SB_CHAR_TYPE__WEAK_LIMIT                \
(                                               \
   SB_CHAR_TYPE__WEAK_END                       \
 - SB_CHAR_TYPE__WEAK_TOP                       \
)

#define SB_CHAR_TYPE__B                         0x0B
#define SB_CHAR_TYPE__S                         0x0C
#define SB_CHAR_TYPE__ON                        0x0D
#define SB_CHAR_TYPE__WS                        0x0E

#define SB_CHAR_TYPE__SEPARATOR_TOP             SB_CHAR_TYPE__CS
#define SB_CHAR_TYPE__SEPARATOR_END             SB_CHAR_TYPE__S
#define SB_CHAR_TYPE__SEPARATOR_LIMIT           \
(                                               \
   SB_CHAR_TYPE__SEPARATOR_END                  \
 - SB_CHAR_TYPE__SEPARATOR_TOP                  \
)

#define SB_CHAR_TYPE__NEUTRAL_TOP               SB_CHAR_TYPE__B
#define SB_CHAR_TYPE__NEUTRAL_END               SB_CHAR_TYPE__WS
#define SB_CHAR_TYPE__NEUTRAL_LIMIT             \
(                                               \
   SB_CHAR_TYPE__NEUTRAL_END                    \
 - SB_CHAR_TYPE__NEUTRAL_TOP                    \
)

#define SB_CHAR_TYPE__LRI                       0x0F
#define SB_CHAR_TYPE__RLI                       0x10
#define SB_CHAR_TYPE__FSI                       0x11
#define SB_CHAR_TYPE__PDI                       0x12

#define SB_CHAR_TYPE__ISO_INIT_TOP              SB_CHAR_TYPE__LRI
#define SB_CHAR_TYPE__ISO_INIT_END              SB_CHAR_TYPE__FSI
#define SB_CHAR_TYPE__ISO_INIT_LIMIT            \
(                                               \
   SB_CHAR_TYPE__ISO_INIT_END                   \
 - SB_CHAR_TYPE__ISO_INIT_TOP                   \
)

#define SB_CHAR_TYPE__ISOLATE_TOP               SB_CHAR_TYPE__ISO_INIT_TOP
#define SB_CHAR_TYPE__ISOLATE_END               SB_CHAR_TYPE__PDI
#define SB_CHAR_TYPE__ISOLATE_LIMIT             \
(                                               \
   SB_CHAR_TYPE__ISOLATE_END                    \
 - SB_CHAR_TYPE__ISOLATE_TOP                    \
)

#define SB_CHAR_TYPE__NTRL_ISO_TOP              SB_CHAR_TYPE__NEUTRAL_TOP
#define SB_CHAR_TYPE__NTRL_ISO_END              SB_CHAR_TYPE__ISOLATE_END
#define SB_CHAR_TYPE__NTRL_ISO_LIMIT            \
(                                               \
   SB_CHAR_TYPE__NTRL_ISO_END                   \
 - SB_CHAR_TYPE__NTRL_ISO_TOP                   \
)

#define SB_CHAR_TYPE__LRE                       0x13
#define SB_CHAR_TYPE__RLE                       0x14
#define SB_CHAR_TYPE__LRO                       0x15
#define SB_CHAR_TYPE__RLO                       0x16
#define SB_CHAR_TYPE__PDF                       0x17

#define SB_CHAR_TYPE__NUMBER_CASE               \
     SB_CHAR_TYPE__EN:                          \
case SB_CHAR_TYPE__AN

#define SB_CHAR_TYPE__NEUTRAL_CASE              \
     SB_CHAR_TYPE__B:                           \
case SB_CHAR_TYPE__S:                           \
case SB_CHAR_TYPE__WS:                          \
case SB_CHAR_TYPE__ON

#define SB_CHAR_TYPE__ISOLATE_INITIATOR_CASE    \
     SB_CHAR_TYPE__LRI:                         \
case SB_CHAR_TYPE__RLI:                         \
case SB_CHAR_TYPE__FSI

#define SB_CHAR_TYPE__ISOLATE_CASE              \
     SB_CHAR_TYPE__ISOLATE_INITIATOR_CASE:      \
case SB_CHAR_TYPE__PDI

#define SB_CHAR_TYPE__ISOLATE_OR_NEUTRAL_CASE   \
     SB_CHAR_TYPE__NEUTRAL_CASE:                \
case SB_CHAR_TYPE__ISOLATE_CASE

#define SB_CHAR_TYPE__WHITESPACE_OR_ISOLATE_CASE\
     SB_CHAR_TYPE__WS:                          \
case SB_CHAR_TYPE__ISOLATE_CASE

#define SB_CHAR_TYPE__EMBEDDING_CASE            \
     SB_CHAR_TYPE__LRE:                         \
case SB_CHAR_TYPE__RLE

#define SB_CHAR_TYPE__OVERRIDE_CASE             \
     SB_CHAR_TYPE__LRO:                         \
case SB_CHAR_TYPE__RLO

#define SB_CHAR_TYPE__EXPLICIT_INITIATOR_CASE   \
     SB_CHAR_TYPE__EMBEDDING_CASE:              \
case SB_CHAR_TYPE__OVERRIDE_CASE

#define SB_CHAR_TYPE__EXPLICIT_CASE             \
     SB_CHAR_TYPE__EXPLICIT_INITIATOR_CASE:     \
case SB_CHAR_TYPE__PDF

#define SB_CHAR_TYPE__BN_EQUIVALENT_CASE        \
     SB_CHAR_TYPE__EXPLICIT_CASE:               \
case SB_CHAR_TYPE__BN

#define SB_CHAR_TYPE__FORMAT_CASE               \
     SB_CHAR_TYPE__ISOLATE_CASE:                \
case SB_CHAR_TYPE__EXPLICIT_CASE

#define SB_CHAR_TYPE__IS_STRONG(t)              \
(                                               \
 (SBUInteger)                                   \
 ((t) - SB_CHAR_TYPE__STRONG_TOP)               \
        <= SB_CHAR_TYPE__STRONG_LIMIT           \
)

#define SB_CHAR_TYPE__IS_NUMBER(t)              \
(                                               \
    (t) == SB_CHAR_TYPE__EN                     \
 || (t) == SB_CHAR_TYPE__AN                     \
)

#define SB_CHAR_TYPE__IS_STRONG_OR_NUMBER(t)    \
(                                               \
 (SBUInteger)                                   \
 ((t) - SB_CHAR_TYPE__STRONG_OR_NUMBER_TOP)     \
        <= SB_CHAR_TYPE__STRONG_OR_NUMBER_LIMIT \
)

#define SB_CHAR_TYPE__IS_SEPARATOR(t)           \
(                                               \
 (SBUInteger)                                   \
 ((t) - SB_CHAR_TYPE__SEPARATOR_TOP)            \
        <= SB_CHAR_TYPE__SEPARATOR_LIMIT        \
)

#define SB_CHAR_TYPE__IS_NEUTRAL(t)             \
(                                               \
 (SBUInteger)                                   \
 ((t) - SB_CHAR_TYPE__NEUTRAL_TOP)              \
        <= SB_CHAR_TYPE__NEUTRAL_LIMIT          \
)

#define SB_CHAR_TYPE__IS_ISOLATE_INITIATOR(t)   \
(                                               \
 (SBUInteger)                                   \
 ((t) - SB_CHAR_TYPE__ISO_INIT_TOP)             \
        <= SB_CHAR_TYPE__ISO_INIT_LIMIT         \
)

#define SB_CHAR_TYPE__IS_ISOLATE_TERMINATOR(t)  \
(                                               \
 (t) == SB_CHAR_TYPE__PDI                       \
)

#define SB_CHAR_TYPE__IS_ISOLATE(t)             \
(                                               \
 (SBUInteger)                                   \
 ((t) - SB_CHAR_TYPE__ISOLATE_TOP)              \
        <= SB_CHAR_TYPE__ISOLATE_LIMIT          \
)

#define SB_CHAR_TYPE__IS_NEUTRAL_OR_ISOLATE(t)  \
(                                               \
 (SBUInteger)                                   \
 ((t) - SB_CHAR_TYPE__NTRL_ISO_TOP)             \
        <= SB_CHAR_TYPE__NTRL_ISO_LIMIT         \
)

enum {
    SBCharTypeNil = SB_CHAR_TYPE__NIL,

    SBCharTypeL   = SB_CHAR_TYPE__L,        /**< strong: left-to-right */
    SBCharTypeR   = SB_CHAR_TYPE__R,        /**< strong: right-to-left */
    SBCharTypeAL  = SB_CHAR_TYPE__AL,       /**< strong: right-to-left Arabic */

    SBCharTypeEN  = SB_CHAR_TYPE__EN,       /**< weak: European Number */
    SBCharTypeES  = SB_CHAR_TYPE__ES,       /**< weak: European Number Separator */
    SBCharTypeET  = SB_CHAR_TYPE__ET,       /**< weak: European Number Terminator */
    SBCharTypeAN  = SB_CHAR_TYPE__AN,       /**< weak: Arabic Number */
    SBCharTypeCS  = SB_CHAR_TYPE__CS,       /**< weak: Common Number Separator */
    SBCharTypeNSM = SB_CHAR_TYPE__NSM,      /**< weak: non-spacing mark */
    SBCharTypeBN  = SB_CHAR_TYPE__BN,       /**< weak: boundary neutral */

    SBCharTypeB   = SB_CHAR_TYPE__B,        /**< neutral: Paragraph Separator */
    SBCharTypeS   = SB_CHAR_TYPE__S,        /**< neutral: Segment Separator */
    SBCharTypeWS  = SB_CHAR_TYPE__WS,       /**< neutral: White Space */
    SBCharTypeON  = SB_CHAR_TYPE__ON,       /**< neutral: Other Neutral */

    SBCharTypeLRE = SB_CHAR_TYPE__LRE,      /**< format: left-to-right embedding */
    SBCharTypeRLE = SB_CHAR_TYPE__RLE,      /**< format: right-to-left embedding */
    SBCharTypeLRO = SB_CHAR_TYPE__LRO,      /**< format: left-to-right override */
    SBCharTypeRLO = SB_CHAR_TYPE__RLO,      /**< format: right-to-left override */
    SBCharTypePDF = SB_CHAR_TYPE__PDF,      /**< format: pop directional formatting */
    SBCharTypeLRI = SB_CHAR_TYPE__LRI,      /**< format: left-to-right isolate */
    SBCharTypeRLI = SB_CHAR_TYPE__RLI,      /**< format: right-to-left isolate */
    SBCharTypeFSI = SB_CHAR_TYPE__FSI,      /**< format: first strong isolate */
    SBCharTypePDI = SB_CHAR_TYPE__PDI       /**< format: pop directional isolate */
};
typedef SB_CHAR_TYPE SBCharType;

#endif
