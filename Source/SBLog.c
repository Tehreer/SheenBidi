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

#include <SBConfig.h>

#ifdef SB_CONFIG_LOG

#include <SBTypes.h>
#include <SBBaseDirection.h>

#include "SBCharType.h"
#include "SBBidiLink.h"
#include "SBIsolatingRun.h"
#include "SBLog.h"

int __SBLogPosition = 0;

SB_INTERNAL void _SBPrintBaseDirection(SBBaseDirection direction) {
    switch (direction) {
        case SBBaseDirectionAutoLTR:
            _SB_LOG_STRING("Auto-LTR");
            break;

        case SBBaseDirectionAutoRTL:
            _SB_LOG_STRING("Auto-RTL");
            break;

        case SBBaseDirectionLTR:
            _SB_LOG_STRING("LTR");
            break;

        case SBBaseDirectionRTL:
            _SB_LOG_STRING("RTL");
            break;
    }
}

SB_INTERNAL void _SBPrintCharType(_SBCharType type) {
    switch (type) {
        case _SBCharTypeNil:
            _SB_LOG_STRING("Nil");
            break;

        case _SBCharTypeL:
            _SB_LOG_STRING("L");
            break;

        case _SBCharTypeR:
            _SB_LOG_STRING("R");
            break;

        case _SBCharTypeAL:
            _SB_LOG_STRING("AL");
            break;

        case _SBCharTypeEN:
            _SB_LOG_STRING("EN");
            break;

        case _SBCharTypeES:
            _SB_LOG_STRING("ES");
            break;

        case _SBCharTypeET:
            _SB_LOG_STRING("EN");
            break;

        case _SBCharTypeAN:
            _SB_LOG_STRING("AN");
            break;

        case _SBCharTypeCS:
            _SB_LOG_STRING("CS");
            break;

        case _SBCharTypeNSM:
            _SB_LOG_STRING("NSM");
            break;

        case _SBCharTypeBN:
            _SB_LOG_STRING("BN");
            break;

        case _SBCharTypeB:
            _SB_LOG_STRING("B");
            break;

        case _SBCharTypeS:
            _SB_LOG_STRING("S");
            break;

        case _SBCharTypeWS:
            _SB_LOG_STRING("WS");
            break;

        case _SBCharTypeON:
            _SB_LOG_STRING("ON");
            break;

        case _SBCharTypeLRE:
            _SB_LOG_STRING("LRE");
            break;

        case _SBCharTypeRLE:
            _SB_LOG_STRING("RLE");
            break;

        case _SBCharTypeLRO:
            _SB_LOG_STRING("LRO");
            break;

        case _SBCharTypeRLO:
            _SB_LOG_STRING("RLO");
            break;

        case _SBCharTypePDF:
            _SB_LOG_STRING("PDF");
            break;

        case _SBCharTypeLRI:
            _SB_LOG_STRING("LRI");
            break;

        case _SBCharTypeRLI:
            _SB_LOG_STRING("RLI");
            break;

        case _SBCharTypeFSI:
            _SB_LOG_STRING("FSI");
            break;

        case _SBCharTypePDI:
            _SB_LOG_STRING("PDI");
            break;
    }
}

SB_INTERNAL void _SBPrintCharactersArray(SBUnichar *characters, SBUInteger length) {
    SBUInteger index;

    for (index = 0; index < length; ++index) {
        _SB_LOG(("%04X ", characters[index]));
    }
}

SB_INTERNAL void _SBPrintCharTypesArray(_SBCharType *types, SBUInteger length) {
    SBUInteger index;

    for (index = 0; index < length; ++index) {
        _SB_LOG_CHAR_TYPE(types[index]);
        _SB_LOG_DIVIDER(1);
    }
}

SB_INTERNAL void _SBPrintLevelsArray(SBLevel *levels, SBUInteger length) {
    SBUInteger index = 0;

    for (index = 0; index < length; ++index) {
        _SB_LOG_LEVEL(levels[index]);
        _SB_LOG_DIVIDER(1);
    }
}

SB_INTERNAL void _SBPrintRunRange(_SBBidiLinkRef roller) {
    _SBBidiLinkRef link;

    SBUInteger offset = 0;
    SBUInteger length = 0;

    for (link = roller->next; link != roller; link = link->next) {
        if (length == 0) {
            offset = link->offset;
            length = link->length;
        } else if (link->offset == (offset + length)) {
            length += link->length;
        } else {
            _SB_LOG_RANGE(offset, length);
            _SB_LOG_DIVIDER(1);

            offset = link->offset;
            length = link->length;
        }
    }

    _SB_LOG_RANGE(offset, length);
}

SB_INTERNAL void _SBPrintLinkTypes(_SBBidiLinkRef roller) {
    _SBBidiLinkRef link;

    for (link = roller->next; link != roller; link = link->next) {
        _SBCharType type = link->type;
        SBUInteger length = link->length;

        while (length--) {
            _SB_LOG_CHAR_TYPE(type);
            _SB_LOG_DIVIDER(1);
        }
    }
}

SB_INTERNAL void _SBPrintLinkLevels(_SBBidiLinkRef roller) {
    _SBBidiLinkRef link;

    for (link = roller->next; link != roller; link = link->next) {
        SBLevel level = link->level;
        SBUInteger length = link->length;

        while (length--) {
            _SB_LOG_LEVEL(level);
            _SB_LOG_DIVIDER(1);
        }
    }
}

#endif
