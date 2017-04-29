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

#include <string>
#include <utility>
#include <ostream>

namespace quick_shell
{
namespace input
{
#error finish
struct SimpleLocation
{
    std::size_t line;
    std::size_t column;
    constexpr SimpleLocation() noexcept : line(1), column(1)
    {
    }
    constexpr SimpleLocation(std::size_t line, std::size_t column) noexcept : line(line),
                                                                              column(column)
    {
    }
    static constexpr std::size_t defaultTabSize = 8;
    static constexpr std::size_t columnAfterTab(std::size_t column, std::size_t tabSize) noexcept
    {
        return tabSize == 0 || column == 0 ? column + 1 :
                                             column + (tabSize - (column - 1) % tabSize);
    }
    constexpr std::size_t columnAfterTab(std::size_t tabSize = defaultTabSize) const noexcept
    {
        return columnAfterTab(column, tabSize);
    }
    constexpr SimpleLocation getNextLocation(char32_t ch,
                                             std::size_t tabSize = defaultTabSize) const noexcept
    {
        return ch == '\n' ? SimpleLocation(line + 1, 1) : ch == '\t' ?
                            SimpleLocation(line, columnAfterTab(tabSize)) :
                            SimpleLocation(line, column + 1);
    }
    friend std::ostream &operator<<(std::ostream &os, const SimpleLocation &v)
    {
        os << v.line << ":" << v.column;
        return os;
    }
};

struct InputFileDescriptor
{
    std::string fileName;
    explicit InputFileDescriptor(std::string fileName) : fileName(std::move(fileName))
    {
    }
};

struct Location : public SimpleLocation
{
    const InputFileDescriptor *file;
    constexpr Location() noexcept : SimpleLocation(), file()
    {
    }
    constexpr Location(std::size_t line,
                       std::size_t column,
                       const InputFileDescriptor *file) noexcept : SimpleLocation(line, column),
                                                                   file(file)
    {
    }
    constexpr explicit Location(const InputFileDescriptor *file) noexcept : SimpleLocation(),
                                                                            file(file)
    {
    }
    constexpr Location(const SimpleLocation &locationInFile,
                       const InputFileDescriptor *file) noexcept : SimpleLocation(locationInFile),
                                                                   file(file)
    {
    }
    constexpr Location getNextLocation(char32_t ch, std::size_t tabSize = defaultTabSize) const
        noexcept
    {
        return Location(SimpleLocation::getNextLocation(ch, tabSize), file);
    }
    friend std::ostream &operator<<(std::ostream &os, const Location &v)
    {
        if(v.file)
            os << v.file->fileName;
        else
            os << "<unknown>";
        os << ":" << static_cast<const SimpleLocation &>(v);
        return os;
    }
};
}
}

#endif /* INPUT_LOCATION_H_ */
