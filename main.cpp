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
#include "parser/parser.h"
#include "parser/text_input.h"
#include "parser/location.h"
#include <iostream>

int main()
{
    using namespace quick_shell;
    using namespace parser;
    TextInputFile textInput("test.sh");
    struct MyParserActions final : public ParserActions
    {
        virtual void handleToken(std::size_t parserLevel,
                                 Location location,
                                 ParserBase::TokenType tokenType,
                                 const std::string &tokenValue) override
        {
            std::cout << "handleToken(" << parserLevel << ", "
                      << ParserBase::getTokenDebugString(location, tokenType, tokenValue) << ")"
                      << std::endl;
        }
    };
    MyParserActions parserActions;
    Parser<MyParserActions, true> parser(parserActions, textInput, &std::cout);
    for(;;)
    {
        switch(parser.parseCommand())
        {
        case ParserBase::ParseCommandResult::NoCommand:
        case ParserBase::ParseCommandResult::Success:
            continue;
        case ParserBase::ParseCommandResult::Quit:
            break;
        }
        break;
    }
}
