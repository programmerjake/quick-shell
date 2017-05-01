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
#include "input/stdin.h"
#include "input/memory.h"
#include <iostream>
#include <sstream>

int main()
{
    using namespace quick_shell;
    auto stdInInput = input::makeStdInTextInput(input::TextInputStyle(), true);
#if 0
    auto &ti = *stdInInput;
#else
    input::MemoryTextInput ti("builtin", input::TextInputStyle(), "abcdefgh\ni\njk\tmn");
#endif
    for(auto i = ti.begin(); i != ti.end(); ++i)
    {
        int ch = *i;
        std::cout << "\n" << ti.getLocation(i) << " " << ch;
        if(ch >= 0x20 && ch < 0x7F)
            std::cout << ' ' << static_cast<char>(ch);
        std::cout << std::endl;
        if(ch == input::eof)
            break;
    }
}
