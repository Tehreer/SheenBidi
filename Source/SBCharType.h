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

#ifndef _SB_CHAR_TYPE_H
#define _SB_CHAR_TYPE_H

#include <SBTypes.h>

#define _SB_CHAR_TYPE                           SBUInt8

#define _SB_CHAR_TYPE__NIL                      0x00

#define _SB_CHAR_TYPE__L                        0x01
#define _SB_CHAR_TYPE__R                        0x02
#define _SB_CHAR_TYPE__AL                       0x03

#define _SB_CHAR_TYPE__STRONG_TOP               _SB_CHAR_TYPE__L
#define _SB_CHAR_TYPE__STRONG_END               _SB_CHAR_TYPE__AL
#define _SB_CHAR_TYPE__STRONG_LIMIT             \
(                                               \
   _SB_CHAR_TYPE__STRONG_END                    \
 - _SB_CHAR_TYPE__STRONG_TOP                    \
)

#define _SB_CHAR_TYPE__EN                       0x04
#define _SB_CHAR_TYPE__AN                       0x05
#define _SB_CHAR_TYPE__ET                       0x06
#define _SB_CHAR_TYPE__NSM                      0x07
#define _SB_CHAR_TYPE__BN                       0x08
#define _SB_CHAR_TYPE__CS                       0x09
#define _SB_CHAR_TYPE__ES                       0x0A

#define _SB_CHAR_TYPE__NUMBER_TOP               _SB_CHAR_TYPE__EN
#define _SB_CHAR_TYPE__NUMBER_END               _SB_CHAR_TYPE__AN
#define _SB_CHAR_TYPE__NUMBER_LIMIT             \
(                                               \
   _SB_CHAR_TYPE__NUMBER_END                    \
 - _SB_CHAR_TYPE__NUMBER_TOP                    \
)

#define _SB_CHAR_TYPE__STRONG_OR_NUMBER_TOP     _SB_CHAR_TYPE__STRONG_TOP
#define _SB_CHAR_TYPE__STRONG_OR_NUMBER_END     _SB_CHAR_TYPE__NUMBER_END
#define _SB_CHAR_TYPE__STRONG_OR_NUMBER_LIMIT   \
(                                               \
  _SB_CHAR_TYPE__STRONG_OR_NUMBER_END           \
- _SB_CHAR_TYPE__STRONG_OR_NUMBER_TOP           \
)

#define _SB_CHAR_TYPE__WEAK_TOP                 _SB_CHAR_TYPE__EN
#define _SB_CHAR_TYPE__WEAK_END                 _SB_CHAR_TYPE__ES
#define _SB_CHAR_TYPE__WEAK_LIMIT               \
(                                               \
   _SB_CHAR_TYPE__WEAK_END                      \
 - _SB_CHAR_TYPE__WEAK_TOP                      \
)

#define _SB_CHAR_TYPE__B                        0x0B
#define _SB_CHAR_TYPE__S                        0x0C
#define _SB_CHAR_TYPE__ON                       0x0D
#define _SB_CHAR_TYPE__WS                       0x0E

#define _SB_CHAR_TYPE__SEPARATOR_TOP            _SB_CHAR_TYPE__CS
#define _SB_CHAR_TYPE__SEPARATOR_END            _SB_CHAR_TYPE__S
#define _SB_CHAR_TYPE__SEPARATOR_LIMIT          \
(                                               \
   _SB_CHAR_TYPE__SEPARATOR_END                 \
 - _SB_CHAR_TYPE__SEPARATOR_TOP                 \
)

#define _SB_CHAR_TYPE__NEUTRAL_TOP              _SB_CHAR_TYPE__B
#define _SB_CHAR_TYPE__NEUTRAL_END              _SB_CHAR_TYPE__WS
#define _SB_CHAR_TYPE__NEUTRAL_LIMIT            \
(                                               \
   _SB_CHAR_TYPE__NEUTRAL_END                   \
 - _SB_CHAR_TYPE__NEUTRAL_TOP                   \
)

#define _SB_CHAR_TYPE__LRI                      0x0F
#define _SB_CHAR_TYPE__RLI                      0x10
#define _SB_CHAR_TYPE__FSI                      0x11
#define _SB_CHAR_TYPE__PDI                      0x12

#define _SB_CHAR_TYPE__ISO_INIT_TOP             _SB_CHAR_TYPE__LRI
#define _SB_CHAR_TYPE__ISO_INIT_END             _SB_CHAR_TYPE__FSI
#define _SB_CHAR_TYPE__ISO_INIT_LIMIT           \
(                                               \
   _SB_CHAR_TYPE__ISO_INIT_END                  \
 - _SB_CHAR_TYPE__ISO_INIT_TOP                  \
)

#define _SB_CHAR_TYPE__ISOLATE_TOP              _SB_CHAR_TYPE__ISO_INIT_TOP
#define _SB_CHAR_TYPE__ISOLATE_END              _SB_CHAR_TYPE__PDI
#define _SB_CHAR_TYPE__ISOLATE_LIMIT            \
(                                               \
   _SB_CHAR_TYPE__ISOLATE_END                   \
 - _SB_CHAR_TYPE__ISOLATE_TOP                   \
)

#define _SB_CHAR_TYPE__NTRL_ISO_TOP             _SB_CHAR_TYPE__NEUTRAL_TOP
#define _SB_CHAR_TYPE__NTRL_ISO_END             _SB_CHAR_TYPE__ISOLATE_END
#define _SB_CHAR_TYPE__NTRL_ISO_LIMIT           \
(                                               \
   _SB_CHAR_TYPE__NTRL_ISO_END                  \
 - _SB_CHAR_TYPE__NTRL_ISO_TOP                  \
)

#define _SB_CHAR_TYPE__LRE                      0x13
#define _SB_CHAR_TYPE__RLE                      0x14
#define _SB_CHAR_TYPE__LRO                      0x15
#define _SB_CHAR_TYPE__RLO                      0x16
#define _SB_CHAR_TYPE__PDF                      0x17

#define _SB_CHAR_TYPE__NUMBER_CASE              \
     _SB_CHAR_TYPE__EN:                         \
case _SB_CHAR_TYPE__AN

#define _SB_CHAR_TYPE__NEUTRAL_CASE             \
     _SB_CHAR_TYPE__B:                          \
case _SB_CHAR_TYPE__S:                          \
case _SB_CHAR_TYPE__WS:                         \
case _SB_CHAR_TYPE__ON

#define _SB_CHAR_TYPE__ISOLATE_INITIATOR_CASE   \
     _SB_CHAR_TYPE__LRI:                        \
case _SB_CHAR_TYPE__RLI:                        \
case _SB_CHAR_TYPE__FSI

#define _SB_CHAR_TYPE__ISOLATE_CASE             \
     _SB_CHAR_TYPE__ISOLATE_INITIATOR_CASE:     \
case _SB_CHAR_TYPE__PDI

#define _SB_CHAR_TYPE__ISOLATE_OR_NEUTRAL_CASE  \
     _SB_CHAR_TYPE__NEUTRAL_CASE:               \
case _SB_CHAR_TYPE__ISOLATE_CASE

#define _SB_CHAR_TYPE__WHITESPACE_OR_ISOLATE_CASE \
     _SB_CHAR_TYPE__WS:                         \
case _SB_CHAR_TYPE__ISOLATE_CASE

#define _SB_CHAR_TYPE__EMBEDDING_CASE           \
     _SB_CHAR_TYPE__LRE:                        \
case _SB_CHAR_TYPE__RLE

#define _SB_CHAR_TYPE__OVERRIDE_CASE            \
     _SB_CHAR_TYPE__LRO:                        \
case _SB_CHAR_TYPE__RLO

#define _SB_CHAR_TYPE__EXPLICIT_INITIATOR_CASE  \
     _SB_CHAR_TYPE__EMBEDDING_CASE:             \
case _SB_CHAR_TYPE__OVERRIDE_CASE

#define _SB_CHAR_TYPE__EXPLICIT_CASE            \
     _SB_CHAR_TYPE__EXPLICIT_INITIATOR_CASE:    \
case _SB_CHAR_TYPE__PDF

#define _SB_CHAR_TYPE__BN_EQUIVALENT_CASE       \
     _SB_CHAR_TYPE__EXPLICIT_CASE:              \
case _SB_CHAR_TYPE__BN

#define _SB_CHAR_TYPE__FORMAT_CASE              \
     _SB_CHAR_TYPE__ISOLATE_CASE:               \
case _SB_CHAR_TYPE__EXPLICIT_CASE

#define _SB_CHAR_TYPE__IS_STRONG(t)             \
(                                               \
 (SBUInteger)                                   \
 ((t) - _SB_CHAR_TYPE__STRONG_TOP)              \
        <= _SB_CHAR_TYPE__STRONG_LIMIT          \
)

#define _SB_CHAR_TYPE__IS_NUMBER(t)             \
(                                               \
    (t) == _SB_CHAR_TYPE__EN                    \
 || (t) == _SB_CHAR_TYPE__AN                    \
)

#define _SB_CHAR_TYPE__IS_STRONG_OR_NUMBER(t)   \
(                                               \
 (SBUInteger)                                   \
 ((t) - _SB_CHAR_TYPE__STRONG_OR_NUMBER_TOP)    \
        <= _SB_CHAR_TYPE__STRONG_OR_NUMBER_LIMIT\
)

#define _SB_CHAR_TYPE__IS_SEPARATOR(t)          \
(                                               \
 (SBUInteger)                                   \
 ((t) - _SB_CHAR_TYPE__SEPARATOR_TOP)           \
        <= _SB_CHAR_TYPE__SEPARATOR_LIMIT       \
)

#define _SB_CHAR_TYPE__IS_NEUTRAL(t)            \
(                                               \
 (SBUInteger)                                   \
 ((t) - _SB_CHAR_TYPE__NEUTRAL_TOP)             \
        <= _SB_CHAR_TYPE__NEUTRAL_LIMIT         \
)

#define _SB_CHAR_TYPE__IS_ISOLATE_INITIATOR(t)  \
(                                               \
 (SBUInteger)                                   \
 ((t) - _SB_CHAR_TYPE__ISO_INIT_TOP)            \
        <= _SB_CHAR_TYPE__ISO_INIT_LIMIT        \
)

#define _SB_CHAR_TYPE__IS_ISOLATE_TERMINATOR(t) \
(                                               \
 (t) == _SB_CHAR_TYPE__PDI                      \
)

#define _SB_CHAR_TYPE__IS_ISOLATE(t)            \
(                                               \
 (SBUInteger)                                   \
 ((t) - _SB_CHAR_TYPE__ISOLATE_TOP)             \
        <= _SB_CHAR_TYPE__ISOLATE_LIMIT         \
)

#define _SB_CHAR_TYPE__IS_NEUTRAL_OR_ISOLATE(t) \
(                                               \
 (SBUInteger)                                   \
 ((t) - _SB_CHAR_TYPE__NTRL_ISO_TOP)            \
        <= _SB_CHAR_TYPE__NTRL_ISO_LIMIT        \
)

enum {
    _SBCharTypeNil = _SB_CHAR_TYPE__NIL,

    _SBCharTypeL   = _SB_CHAR_TYPE__L,      /**< strong: left-to-right */
    _SBCharTypeR   = _SB_CHAR_TYPE__R,      /**< strong: right-to-left */
    _SBCharTypeAL  = _SB_CHAR_TYPE__AL,     /**< strong: right-to-left Arabic */

    _SBCharTypeEN  = _SB_CHAR_TYPE__EN,     /**< weak: European Number */
    _SBCharTypeES  = _SB_CHAR_TYPE__ES,     /**< weak: European Number Separator */
    _SBCharTypeET  = _SB_CHAR_TYPE__ET,     /**< weak: European Number Terminator */
    _SBCharTypeAN  = _SB_CHAR_TYPE__AN,     /**< weak: Arabic Number */
    _SBCharTypeCS  = _SB_CHAR_TYPE__CS,     /**< weak: Common Number Separator */
    _SBCharTypeNSM = _SB_CHAR_TYPE__NSM,    /**< weak: non-spacing mark */
    _SBCharTypeBN  = _SB_CHAR_TYPE__BN,     /**< weak: boundary neutral */

    _SBCharTypeB   = _SB_CHAR_TYPE__B,      /**< neutral: Paragraph Separator */
    _SBCharTypeS   = _SB_CHAR_TYPE__S,      /**< neutral: Segment Separator */
    _SBCharTypeWS  = _SB_CHAR_TYPE__WS,     /**< neutral: White Space */
    _SBCharTypeON  = _SB_CHAR_TYPE__ON,     /**< neutral: Other Neutral */

    _SBCharTypeLRE = _SB_CHAR_TYPE__LRE,    /**< format: left-to-right embedding */
    _SBCharTypeRLE = _SB_CHAR_TYPE__RLE,    /**< format: right-to-left embedding */
    _SBCharTypeLRO = _SB_CHAR_TYPE__LRO,    /**< format: left-to-right override */
    _SBCharTypeRLO = _SB_CHAR_TYPE__RLO,    /**< format: right-to-left override */
    _SBCharTypePDF = _SB_CHAR_TYPE__PDF,    /**< format: pop directional formatting */
    _SBCharTypeLRI = _SB_CHAR_TYPE__LRI,    /**< format: left-to-right isolate */
    _SBCharTypeRLI = _SB_CHAR_TYPE__RLI,    /**< format: right-to-left isolate */
    _SBCharTypeFSI = _SB_CHAR_TYPE__FSI,    /**< format: first strong isolate */
    _SBCharTypePDI = _SB_CHAR_TYPE__PDI     /**< format: pop directional isolate */
};
typedef _SB_CHAR_TYPE _SBCharType;

#endif
