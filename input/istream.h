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
#ifndef INPUT_ISTREAM_H_
#define INPUT_ISTREAM_H_

#include <iosfwd>
#include "text_input.h"

namespace quick_shell
{
namespace input
{
class IStreamTextInput final : public TextInput
{
private:
    std::istream &is;

protected:
    virtual std::size_t read(std::size_t startIndex,
                             unsigned char *buffer,
                             std::size_t bufferSize) override;

public:
    IStreamTextInput(std::string name,
                     const TextInputStyle &inputStyle,
                     std::istream &is,
                     bool retryAfterEOF)
        : TextInput(std::move(name), inputStyle, retryAfterEOF), is(is)
    {
    }
};
}
}

#endif /* INPUT_ISTREAM_H_ */
