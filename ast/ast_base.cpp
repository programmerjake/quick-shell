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
#include "ast_base.h"
#include <ostream>

namespace quick_shell
{
namespace ast
{
std::ostream &operator<<(std::ostream &os, const ASTDumpState::IndentState &indentState)
{
    for(std::size_t i = 0; i < indentState.indentDepth; i++)
        os << indentState.indentString;
    return os;
}

std::ostream &operator<<(std::ostream &os, const ASTDumpState::EscapedQuotedString &v)
{
    os << '\"';
    for(unsigned char ch : v.string)
    {
        switch(ch)
        {
        case '\'':
        case '\"':
        case '\\':
        	os << '\\' << ch;
        	continue;
        case '\a':
            os << "\\a";
            continue;
        case '\b':
            os << "\\b";
            continue;
        case '\f':
            os << "\\f";
            continue;
        case '\n':
            os << "\\n";
            continue;
        case '\r':
            os << "\\r";
            continue;
        case '\t':
            os << "\\t";
            continue;
        case '\v':
            os << "\\v";
            continue;
        default:
            if(ch < 0x20 || ch >= 0x7F)
            {
                os << "\\x";
                auto previousWidth = os.width();
                auto previousFill = os.fill();
                auto previousFlags = os.flags();
                os.width(2);
                os.fill('0');
                os << std::hex << std::uppercase;
                os << static_cast<unsigned>(ch & 0xFF);
                os.width(previousWidth);
                os.fill(previousFill);
                os.flags(previousFlags);
                continue;
            }
            os << ch;
            continue;
        }
    }
    os << '\"';
    return os;
}
}
}
