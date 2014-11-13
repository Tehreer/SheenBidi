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

#ifndef _SB_LOG_H
#define _SB_LOG_H

#include <SBConfig.h>

#ifdef SB_CONFIG_LOG

#include <stdio.h>

#include <SBTypes.h>
#include <SBBaseDirection.h>

#include "SBCharType.h"
#include "SBRunLink.h"
#include "SBIsolatingRun.h"

SB_INTERNAL void _SBPrintBaseDirection(SBBaseDirection direction);
SB_INTERNAL void _SBPrintCharType(_SBCharType type);

SB_INTERNAL void _SBPrintCharactersArray(SBUnichar *characters, SBUInteger length);
SB_INTERNAL void _SBPrintCharTypesArray(_SBCharType *types, SBUInteger length);
SB_INTERNAL void _SBPrintLevelsArray(SBLevel *levels, SBUInteger length);

SB_INTERNAL void _SBPrintLinkTypes(_SBRunLinkIter iter);
SB_INTERNAL void _SBPrintLinkLevels(_SBRunLinkIter iter);

SB_INTERNAL void _SBPrintIsolatingRunRange(_SBIsolatingRunRef isolatingRun);

extern int __SBLogPosition;

#define _SB_LOG_BEGIN()                 (++__SBLogPosition)
#define _SB_LOG_END()                   (--__SBLogPosition)

#define _SB_LOG(s)                      printf s

#define _SB_LOG_NUMBER(n)               \
_SB_LOG(("%ld", (long)n))

#define _SB_LOG_RANGE(o, l)             \
_SB_LOG(("[%ld, %ld]", (long)o, (long)(o + l - 1)))

#define _SB_LOG_CHAR(c)                 \
SBLOG(("%c", c))

#define _SB_LOG_STRING(s)               \
_SB_LOG(("%s", s))

#define _SB_LOG_LEVEL(l)                \
_SB_LOG_NUMBER(l)

#define _SB_LOG_BREAKER()               \
_SB_LOG(("\n"))

#define _SB_LOG_DIVIDER(n)              \
_SB_LOG(("%.*s", n, "\t\t\t\t\t\t\t\t\t\t"))

#define _SB_LOG_INITIATOR()             \
_SB_LOG_DIVIDER(__SBLogPosition)

#define _SB_LOG_CAPTION(c)              \
_SB_LOG((c":"))

#define _SB_LOG_STATEMENT_TEXT(t)       \
(t)

#define _SB_LOG_LINE(s)                 \
do {                                    \
    _SB_LOG(s);                         \
    _SB_LOG_BREAKER();                  \
} while (0)

#define _SB_LOG_STATEMENT(c, d, t)      \
do {                                    \
    _SB_LOG_INITIATOR();                \
    _SB_LOG_CAPTION(c);                 \
    _SB_LOG_DIVIDER(d);                 \
    _SB_LOG_STATEMENT_TEXT(t);          \
    _SB_LOG_BREAKER();                  \
} while (0)

#define _SB_LOG_BLOCK_OPENER(c)         \
do {                                    \
    _SB_LOG_INITIATOR();                \
    _SB_LOG_CAPTION(c);                 \
    _SB_LOG_BREAKER();                  \
    _SB_LOG_BEGIN();                    \
} while (0)

#define _SB_LOG_BLOCK_CLOSER()          \
_SB_LOG_END()

#define _SB_LOG_BASE_DIRECTION(d)       \
_SBPrintBaseDirection(d)

#define _SB_LOG_CHAR_TYPE(t)            \
_SBPrintCharType(t)

#define _SB_LOG_CHARACTERS_ARRAY(a, l)  \
_SBPrintCharactersArray(a, l)

#define _SB_LOG_CHAR_TYPES_ARRAY(a, l)  \
_SBPrintCharTypesArray(a, l)

#define _SB_LOG_LEVELS_ARRAY(a, l)      \
_SBPrintLevelsArray(a, l)

#define _SB_LOG_LINK_TYPES(c)           \
_SBPrintLinkTypes(c)

#define _SB_LOG_LINK_LEVELS(c)          \
_SBPrintLinkLevels(c)

#define _SB_LOG_ISOLATING_RUN_RANGE(ir) \
_SBPrintIsolatingRunRange(ir)

#else

#define _SB_LOG_NONE()

#define _SB_LOG(s)                      _SB_LOG_NONE()

#define _SB_LOG_NUMBER(n)               _SB_LOG_NONE()
#define _SB_LOG_RANGE(o, l)             _SB_LOG_NONE()
#define _SB_LOG_CHAR(c)                 _SB_LOG_NONE()
#define _SB_LOG_STRING(s)               _SB_LOG_NONE()
#define _SB_LOG_LEVEL(l)                _SB_LOG_NONE()

#define _SB_LOG_BREAKER()               _SB_LOG_NONE()
#define _SB_LOG_DIVIDER(n)              _SB_LOG_NONE()
#define _SB_LOG_INITIATOR()             _SB_LOG_NONE()
#define _SB_LOG_CAPTION(c)              _SB_LOG_NONE()
#define _SB_LOG_STATEMENT_TEXT(t)       _SB_LOG_NONE()

#define _SB_LOG_LINE(s)                 _SB_LOG_NONE()
#define _SB_LOG_STATEMENT(c, d, t)      _SB_LOG_NONE()

#define _SB_LOG_BLOCK_OPENER(c)         _SB_LOG_NONE()
#define _SB_LOG_BLOCK_CLOSER()          _SB_LOG_NONE()

#define _SB_LOG_BASE_DIRECTION(d)       _SB_LOG_NONE()
#define _SB_LOG_CHAR_TYPE(t)            _SB_LOG_NONE()

#define _SB_LOG_CHARACTERS_ARRAY(a, l)  _SB_LOG_NONE()
#define _SB_LOG_CHAR_TYPES_ARRAY(a, l)  _SB_LOG_NONE()
#define _SB_LOG_LEVELS_ARRAY(a, l)      _SB_LOG_NONE()

#define _SB_LOG_LINK_TYPES(c)          _SB_LOG_NONE()
#define _SB_LOG_LINK_LEVELS(c)         _SB_LOG_NONE()

#define _SB_LOG_ISOLATING_RUN_RANGE(ir) _SB_LOG_NONE()

#endif

#endif
