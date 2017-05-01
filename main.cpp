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
#include <iostream>

int main()
{
    using namespace quick_shell;
    auto stdInInput = input::makeStdInTextInput(input::TextInputStyle(), true);
    int eofCount = 0;
    for(auto i = stdInInput->begin(); i != stdInInput->end(); ++i)
    {
        int ch = *i;
        if(ch == input::eof)
        {
            eofCount++;
            if(eofCount > 5)
                break;
        }
        std::cout << "\n" << stdInInput->getLocation(i) << " " << ch;
        if(ch >= 0x20 && ch < 0x7F)
            std::cout << ' ' << static_cast<char>(ch);
        std::cout << std::endl;
    }
}
