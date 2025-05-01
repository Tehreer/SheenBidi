/*
 * Copyright (C) 2025 Muhammad Tayyab Akram
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
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>

#include "UnicodeVersion.h"
#include "DataFile.h"

using namespace std;
using namespace SheenBidi::Parser;

static const string FIELD_TERMINATORS = " ;";

DataFile::DataFile(const string &directory, const string &fileName) {
    auto filePath = directory + "/" + fileName;
    m_stream.open(filePath, ios::in);
    if (!m_stream.is_open()) {
        throw runtime_error("Failed to open file: " + filePath);
    }
}

DataFile::~DataFile() {
    if (m_stream.is_open()) {
        m_stream.close();
    }
}

bool DataFile::readLine(string &line) {
    getline(m_stream, line);
    return m_stream.good();
}

void DataFile::getVersion(const string &line, UnicodeVersion &version) {
    version = UnicodeVersion(line);
}

size_t DataFile::getField(const string &line, size_t index, FieldTerminator terminator, string &field) {
    field.clear();

    auto start = line.find_first_not_of(' ', index);
    if (start == std::string::npos) {
        return line.length();
    }

    size_t end = start;
    switch (terminator) {
    case FieldTerminator::Space:
        end = line.find(' ', start);
        break;
    case FieldTerminator::Semicolon:
        end = line.find(';', start);
        break;
    case FieldTerminator::SpaceOrSemicolon:
        end = line.find_first_of(" ;", start);
        break;
    }

    if (end == std::string::npos) {
        end = line.length();
    }

    field.assign(line, start, end - start);

    auto next = line.find_first_not_of(FIELD_TERMINATORS, end);
    if (next == std::string::npos) {
        return end;
    }

    return next;
}

size_t DataFile::getCodePoint(const string &line, size_t index, uint32_t &codePoint) {
    if (index >= line.size()) {
        throw out_of_range("index");
    }

    auto start = line.c_str() + index;
    char *end = nullptr;

    codePoint = strtoul(start, &end, 16);
    auto length = static_cast<size_t>(end - start);

    return index + length;
}

size_t DataFile::getCodePointRange(const string &line, size_t index, uint32_t &first, uint32_t &last) {
    index = getCodePoint(line, index, first);

    if (line.at(index) == '.') {
        index = line.find_first_not_of('.', index);
        index = getCodePoint(line, index, last);
    } else {
        last = first;
    }

    auto next = line.find_first_not_of(FIELD_TERMINATORS, index);
    if (next == string::npos) {
        return index;
    }

    return next;
}
