/*
 * Copyright (C) 2015 Muhammad Tayyab Akram
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

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "UnicodeVersion.h"
#include "BidiMirroring.h"

using namespace std;
using namespace SheenBidi::Parser;

static const string FILE_BIDI_MIRRORING = "BidiMirroring.txt";

BidiMirroring::BidiMirroring(const string &directory) :
    m_firstCodePoint(0),
    m_lastCodePoint(0),
    m_mirrors(0xFFFF)
{
    ifstream stream(directory + "/" + FILE_BIDI_MIRRORING, ios::binary);

    string versionLine;
    getline(stream, versionLine);
    m_version = new UnicodeVersion(versionLine);

    char ch;

    while (!stream.eof()) {
        stream >> ch;

        if (ch != '#') {
            uint32_t codePoint;
            uint32_t mirror;

            stream.seekg(-1, ios_base::cur);

            stream >> hex >> setw(4) >> codePoint;
            stream.ignore(4, ';');

            stream >> hex >> setw(4) >> mirror;

            m_mirrors[codePoint] = mirror;

            if (!m_firstCodePoint) {
                m_firstCodePoint = codePoint;
            }

            if (codePoint > m_lastCodePoint) {
                m_lastCodePoint = codePoint;
            }
        }

        stream.ignore(1024, '\n');
    }
}

BidiMirroring::~BidiMirroring() {
    delete m_version;
}

uint32_t BidiMirroring::firstCodePoint() const {
    return m_firstCodePoint;
}

uint32_t BidiMirroring::lastCodePoint() const {
    return m_lastCodePoint;
}

UnicodeVersion &BidiMirroring::version() const {
    return *m_version;
}

uint32_t BidiMirroring::mirrorForCodePoint(uint32_t codePoint) const {
    if (codePoint <= m_lastCodePoint) {
        return m_mirrors.at(codePoint);
    }

    return 0;
}
