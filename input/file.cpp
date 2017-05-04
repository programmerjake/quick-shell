/*
 * Copyright 2017 Jacob Lifshay
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "file.h"
#include "istream.h"
#include <fstream>

namespace quick_shell
{
namespace input
{
struct FileTextInput::Implementation final
{
    std::ifstream is;
    explicit Implementation(util::string_view fileName) : is()
    {
        is.exceptions(std::ios::badbit | std::ios::failbit);
        is.open(static_cast<std::string>(fileName), std::ios::in | std::ios::binary);
    }
};

std::size_t FileTextInput::read(std::size_t startIndex,
                                unsigned char *buffer,
                                std::size_t bufferSize)
{
    return IStreamTextInput::read(implementation->is, startIndex, buffer, bufferSize);
}

FileTextInput::FileTextInput(util::string_view name,
                             util::string_view fileName,
                             const TextInputStyle &inputStyle,
                             bool retryAfterEOF)
    : TextInput(static_cast<std::string>(name), inputStyle, retryAfterEOF),
      implementation(std::make_shared<Implementation>(fileName))
{
}
}
}
