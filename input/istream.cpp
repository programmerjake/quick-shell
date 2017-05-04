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
#include "istream.h"
#include <istream>
#include <cassert>

namespace quick_shell
{
namespace input
{
std::size_t IStreamTextInput::read(std::istream &is,
                                   std::size_t startIndex,
                                   unsigned char *buffer,
                                   std::size_t bufferSize)
{
    assert(bufferSize >= 0);
    if(!is.fail() && is.eof())
    {
        is.clear();
        return 0;
    }
    is.peek(); // sets eof flag without setting fail flag
    if(!is.fail() && is.eof())
    {
        is.clear();
        return 0;
    }
    if(!is.good())
    {
        return 0;
    }
    buffer[0] = is.get();
    if(is.fail())
    {
        return 0;
    }
    if(is.eof())
    {
        is.clear();
        return 0;
    }
    auto readCount = is.readsome(reinterpret_cast<char *>(buffer) + 1, bufferSize - 1);
    return readCount + 1;
}

std::size_t IStreamTextInput::read(std::size_t startIndex,
                                   unsigned char *buffer,
                                   std::size_t bufferSize)
{
    return read(is, startIndex, buffer, bufferSize);
}
}
}
