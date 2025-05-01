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

#ifndef SHEENBIDI_PARSER_DATA_FILE_H
#define SHEENBIDI_PARSER_DATA_FILE_H

#include <fstream>
#include <cstdint>
#include <string>

#include "UnicodeVersion.h"

namespace SheenBidi {
namespace Parser {

class DataFile {
protected:
    static constexpr uint32_t MAX_CODE_POINTS = 0x110000;

    enum class FieldTerminator {
        Space,              // Field ends at first space
        Semicolon,          // Field ends at first semicolon
        SpaceOrSemicolon    // Field ends at either space or semicolon
    };

    DataFile(const std::string &directory, const std::string &fileName);
    virtual ~DataFile();

    bool readLine(std::string &line);

    void getVersion(const std::string &line, UnicodeVersion &version);
    size_t getField(const std::string &line, size_t index, FieldTerminator terminator, std::string &field);
    size_t getCodePoint(const std::string &line, size_t index, uint32_t &codePoint);
    size_t getCodePointRange(const std::string &line, size_t index, uint32_t &first, uint32_t &last);

private:
    std::ifstream m_stream;
};

}
}

#endif
