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

#include "location.h"
#include "input.h"
#include <ostream>

namespace quick_shell
{
namespace input
{
std::ostream &operator<<(std::ostream &os, const SimpleLocation &v)
{
    os << "<byte=" << v.index << ">";
    return os;
}

std::ostream &operator<<(std::ostream &os, const Location &v)
{
    if(v.input)
        os << v.input->getName() << ':' << v.input->getLineAndColumn(v.index);
    else
        os << "<unknown>:" << static_cast<const SimpleLocation &>(v);
    return os;
}
}
}
