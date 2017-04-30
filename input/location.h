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

namespace quick_shell
{
namespace input
{
struct Input;

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

struct Location : public SimpleLocation
{
    Input *input;
    constexpr Location() noexcept : SimpleLocation(), input()
    {
    }
    constexpr Location(std::size_t index, Input *input) noexcept : SimpleLocation(index),
                                                                   input(input)
    {
    }
    constexpr Location(std::size_t index, Input &input) noexcept : SimpleLocation(index),
                                                                   input(&input)
    {
    }
    constexpr explicit Location(Input *input) noexcept : SimpleLocation(), input(input)
    {
    }
    constexpr explicit Location(Input &input) noexcept : SimpleLocation(), input(&input)
    {
    }
    constexpr Location(const SimpleLocation &locationInInput, Input *input) noexcept
        : SimpleLocation(locationInInput),
          input(input)
    {
    }
    friend std::ostream &operator<<(std::ostream &os, const Location &v);
};
}
}

#endif /* INPUT_LOCATION_H_ */
