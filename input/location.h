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
#ifndef INPUT_LOCATION_H_
#define INPUT_LOCATION_H_

#include <iosfwd>
#include <string>

namespace quick_shell
{
namespace input
{
struct TextInput;

struct SimpleLocation
{
    std::size_t index;
    constexpr SimpleLocation() noexcept : index(0)
    {
    }
    constexpr SimpleLocation(std::size_t index) noexcept : index(index)
    {
    }
    friend std::ostream &operator<<(std::ostream &os, const SimpleLocation &v);
};

struct SimpleLocationSpan
{
    /** inclusive */
    std::size_t beginIndex;
    /** exclusive */
    std::size_t endIndex;
    constexpr SimpleLocationSpan() noexcept : beginIndex(0), endIndex(0)
    {
    }
    constexpr SimpleLocationSpan(std::size_t beginIndex, std::size_t endIndex) noexcept
        : beginIndex(beginIndex),
          endIndex(endIndex)
    {
    }
    constexpr std::size_t size() const noexcept
    {
        return endIndex - beginIndex;
    }
    constexpr SimpleLocation begin() const noexcept
    {
        return SimpleLocation(beginIndex);
    }
    constexpr SimpleLocation back() const noexcept
    {
        return SimpleLocation(endIndex - 1);
    }
    constexpr SimpleLocation end() const noexcept
    {
        return SimpleLocation(endIndex);
    }
    constexpr operator SimpleLocation() const noexcept
    {
        return begin();
    }
    friend std::ostream &operator<<(std::ostream &os, const SimpleLocationSpan &v);
};

struct Location : public SimpleLocation
{
    TextInput *input;
    constexpr Location() noexcept : SimpleLocation(), input()
    {
    }
    constexpr Location(std::size_t index, TextInput *input) noexcept : SimpleLocation(index),
                                                                       input(input)
    {
    }
    constexpr Location(std::size_t index, TextInput &input) noexcept : SimpleLocation(index),
                                                                       input(&input)
    {
    }
    constexpr explicit Location(TextInput *input) noexcept : SimpleLocation(), input(input)
    {
    }
    constexpr explicit Location(TextInput &input) noexcept : SimpleLocation(), input(&input)
    {
    }
    constexpr Location(const SimpleLocation &locationInInput, TextInput *input) noexcept
        : SimpleLocation(locationInInput),
          input(input)
    {
    }
    friend std::ostream &operator<<(std::ostream &os, const Location &v);
};

struct LocationSpan : public SimpleLocationSpan
{
    TextInput *input;
    constexpr LocationSpan() noexcept : SimpleLocationSpan(), input()
    {
    }
    constexpr LocationSpan(std::size_t beginIndex, std::size_t endIndex, TextInput *input) noexcept
        : SimpleLocationSpan(beginIndex, endIndex),
          input(input)
    {
    }
    constexpr LocationSpan(std::size_t beginIndex, std::size_t endIndex, TextInput &input) noexcept
        : SimpleLocationSpan(beginIndex, endIndex),
          input(&input)
    {
    }
    constexpr explicit LocationSpan(TextInput *input) noexcept : SimpleLocationSpan(), input(input)
    {
    }
    constexpr explicit LocationSpan(TextInput &input) noexcept : SimpleLocationSpan(), input(&input)
    {
    }
    constexpr LocationSpan(const SimpleLocationSpan &locationSpanInInput, TextInput *input) noexcept
        : SimpleLocationSpan(locationSpanInInput),
          input(input)
    {
    }
    constexpr Location begin() const noexcept
    {
        return Location(beginIndex, input);
    }
    constexpr Location back() const noexcept
    {
        return Location(endIndex - 1, input);
    }
    constexpr Location end() const noexcept
    {
        return Location(endIndex, input);
    }
    constexpr operator Location() const noexcept
    {
        return begin();
    }
    std::string getTextInputText(std::string bufferSource, char replacementForEOF = '\0') const;
    std::string getTextInputText(char replacementForEOF = '\0') const
    {
        return getTextInputText(std::string(), replacementForEOF);
    }
    friend std::ostream &operator<<(std::ostream &os, const LocationSpan &v);
};
}
}

#endif /* INPUT_LOCATION_H_ */
