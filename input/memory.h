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
#ifndef INPUT_MEMORY_H_
#define INPUT_MEMORY_H_

#include <vector>
#include <memory>
#include "text_input.h"
#include "../util/string_view.h"

namespace quick_shell
{
namespace input
{
class MemoryTextInput final : public TextInput
{
protected:
    virtual std::size_t read(std::size_t startIndex,
                             unsigned char *buffer,
                             std::size_t bufferSize) override;

public:
    MemoryTextInput(std::string name,
                    const TextInputStyle &inputStyle,
                    std::shared_ptr<const unsigned char> memory,
                    std::size_t memorySize)
        : TextInput(std::move(name),
                    inputStyle,
                    std::move(memory),
                    memorySize,
                    std::set<std::size_t>(),
                    false)
    {
    }
    MemoryTextInput(std::string name,
                    const TextInputStyle &inputStyle,
                    const unsigned char *memory,
                    std::size_t memorySize)
        : TextInput(std::move(name), inputStyle, memory, memorySize, std::set<std::size_t>(), false)
    {
    }
    MemoryTextInput(std::string name,
                    const TextInputStyle &inputStyle,
                    const std::shared_ptr<const char> &memory,
                    std::size_t memorySize)
        : MemoryTextInput(std::move(name),
                          inputStyle,
                          std::shared_ptr<const unsigned char>(
                              memory, reinterpret_cast<const unsigned char *>(memory.get())),
                          memorySize)
    {
    }
    MemoryTextInput(std::string name,
                    const TextInputStyle &inputStyle,
                    const char *memory,
                    std::size_t memorySize)
        : MemoryTextInput(std::move(name),
                          inputStyle,
                          reinterpret_cast<const unsigned char *>(memory),
                          memorySize)
    {
    }
    MemoryTextInput(std::string name, const TextInputStyle &inputStyle, util::string_view memory)
        : MemoryTextInput(std::move(name), inputStyle, memory.data(), memory.size())
    {
    }
    MemoryTextInput(std::string name,
                    const TextInputStyle &inputStyle,
                    const std::vector<char> &memory)
        : MemoryTextInput(std::move(name), inputStyle, memory.data(), memory.size())
    {
    }
    MemoryTextInput(std::string name,
                    const TextInputStyle &inputStyle,
                    const std::vector<unsigned char> &memory)
        : MemoryTextInput(std::move(name), inputStyle, memory.data(), memory.size())
    {
    }
};
}
}

#endif /* INPUT_MEMORY_H_ */
