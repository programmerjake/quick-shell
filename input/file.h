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
#ifndef INPUT_FILE_H_
#define INPUT_FILE_H_

#include <memory>
#include "text_input.h"
#include "../util/string_view.h"

namespace quick_shell
{
namespace input
{
class FileTextInput final : public TextInput
{
private:
    struct Implementation;

private:
    std::shared_ptr<Implementation> implementation;

protected:
    virtual std::size_t read(std::size_t startIndex,
                             unsigned char *buffer,
                             std::size_t bufferSize) override;

public:
    FileTextInput(util::string_view name,
                  util::string_view fileName,
                  const TextInputStyle &inputStyle = TextInputStyle(),
                  bool retryAfterEOF = false);
    explicit FileTextInput(util::string_view name,
                           const TextInputStyle &inputStyle = TextInputStyle(),
                           bool retryAfterEOF = false)
        : FileTextInput(name, name, inputStyle, retryAfterEOF)
    {
    }
};
}
}

#endif /* INPUT_FILE_H_ */
