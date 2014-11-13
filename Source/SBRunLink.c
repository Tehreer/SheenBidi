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
#include <SBConfig.h>

#include "SBAssert.h"
#include "SBRunLink.h"

SB_INTERNAL void _SBRunLinkMakeEmpty(_SBRunLinkRef link) {
    link->next = NULL;
    link->offset = SBInvalidIndex;
    link->length = 0;
    link->type = _SB_CHAR_TYPE__NIL;
    link->level = SBInvalidLevel;
}

SB_INTERNAL void _SBRunLinkAbandonNext(_SBRunLinkRef link) {
    link->next = link->next->next;
}

SB_INTERNAL void _SBRunLinkReplaceNext(_SBRunLinkRef link, _SBRunLinkRef next) {
    link->next = next;
}

SB_INTERNAL void _SBRunLinkMergeNext(_SBRunLinkRef link) {
    _SBRunLinkRef firstNext;
    _SBRunLinkRef secondNext;

    firstNext = link->next;
    secondNext = firstNext->next;

    link->next = secondNext;
    link->length += firstNext->length;
}

SB_INTERNAL _SBRunLinkIter _SBRunLinkIterMake(_SBRunLinkRef firstLink, _SBRunLinkRef lastLink) {
    _SBRunLinkIter iter;
    iter._first = firstLink;
    iter._break = lastLink->next;
    iter.current = NULL;

    return iter;
}

SB_INTERNAL void _SBRunLinkIterReset(_SBRunLinkIterRef iter) {
    iter->current = NULL;
}

SB_INTERNAL SBBoolean _SBRunLinkIterMoveNext(_SBRunLinkIterRef iter) {
    _SBRunLinkRef current = iter->current;
    return ((iter->current = (current ? current->next : iter->_first)) != iter->_break);
}
