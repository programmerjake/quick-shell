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
#ifndef PARSER_PARSER_H_
#define PARSER_PARSER_H_

#include <utility>
#include "text_input.h"

namespace quick_shell
{
namespace parser
{
struct ParserActions
{
    virtual ~ParserActions() = default;
};

template <typename ParserActions = ParserActions>
class Parser final
{
private:
    ParserActions &actions;
    TextInput &ti;
    enum class TokenType
	{
    	EndOfFile,
		Newline,
		Blanks,
		Comment,
		SingleQuote,
		DollarSingleQuoteStart,
		DollarSingleQuoteEnd,
		UnquotedWordPart,
		QuotedWordPart,
		UnquotedSimpleSubstitution,
		QuotedSimpleSubstitution,
		UnquotedSubstitutionStart,
		QuotedSubstitutionStart,
		UnquotedShellSubstitutionStart,
		UnquotedShellSubstitutionStart,
        ExMark, // "!"
        LBrace, // "{"
        RBrace, // "}"
        DoubleLBracket, // "[["
        DoubleRBracket, // "]]"
        Case, // "case"
        Coproc, // "coproc"
        Do, // "do"
        Done, // "done"
        ElIf, // "elif"
        Else, // "else"
        Esac, // "esac"
        Fi, // "fi"
        For, // "for"
        Function, // "function"
        If, // "if"
        In, // "in"
        Select, // "select"
        Time, // "time"
        Then, // "then"
        Until, // "until"
        While, // "while"
	};
    enum class TokenizerMode
	{
    	Command,
		CommandWithReservedWords,
		DoubleQuotes,
		VariableSubstitution,
		Arithmetic,
		Condition,
		RegEx,
		RegExCharacterClass,
	};
    Location tokenLocation{};
    TokenType tokenType = TokenType::EndOfFile;
    std::string tokenValue{};

public:
    ParserActions &getActions() noexcept
    {
        return actions;
    }
    TextInput &getTextInput() noexcept
    {
        return ti;
    }

public:
    constexpr Parser(ParserActions &actions, TextInput &textInput) : actions(actions), ti(textInput)
    {
    }

private:
    static constexpr bool isBlank(int ch) noexcept
    {
        return ch == ' ' || ch == '\t';
    }
    static constexpr bool isNewline(int ch) noexcept
    {
        return ch == '\n';
    }
    static constexpr bool isMetacharacter(int ch) noexcept
    {
        return ch == '|' || ch == '&' || ch == ';' || ch == '(' || ch == ')' || ch == '<'
               || ch == '>' || isBlank(ch) || isNewline(ch);
    }
    static constexpr bool isMetacharacterOrEOF(int ch) noexcept
    {
        return isMetacharacter(ch) || ch == TextInput::eof;
    }
    enum class ReservedWordKind
    {
        NotAReservedWord,
        ExMark, // "!"
        LBrace, // "{"
        RBrace, // "}"
        DoubleLBracket, // "[["
        DoubleRBracket, // "]]"
        Case, // "case"
        Coproc, // "coproc"
        Do, // "do"
        Done, // "done"
        ElIf, // "elif"
        Else, // "else"
        Esac, // "esac"
        Fi, // "fi"
        For, // "for"
        Function, // "function"
        If, // "if"
        In, // "in"
        Select, // "select"
        Time, // "time"
        Then, // "then"
        Until, // "until"
        While, // "while"
    };
    static util::string_view getReservedWordString(ReservedWordKind reservedWord) noexcept
    {
        switch(reservedWord)
        {
        case ReservedWordKind::NotAReservedWord:
            return "";
        case ReservedWordKind::ExMark:
            return "!";
        case ReservedWordKind::LBrace:
            return "{";
        case ReservedWordKind::RBrace:
            return "}";
        case ReservedWordKind::DoubleLBracket:
            return "[[";
        case ReservedWordKind::DoubleRBracket:
            return "]]";
        case ReservedWordKind::Case:
            return "case";
        case ReservedWordKind::Coproc:
            return "coproc";
        case ReservedWordKind::Do:
            return "do";
        case ReservedWordKind::Done:
            return "done";
        case ReservedWordKind::ElIf:
            return "elif";
        case ReservedWordKind::Else:
            return "else";
        case ReservedWordKind::Esac:
            return "esac";
        case ReservedWordKind::Fi:
            return "fi";
        case ReservedWordKind::For:
            return "for";
        case ReservedWordKind::Function:
            return "function";
        case ReservedWordKind::If:
            return "if";
        case ReservedWordKind::In:
            return "in";
        case ReservedWordKind::Select:
            return "select";
        case ReservedWordKind::Time:
            return "time";
        case ReservedWordKind::Then:
            return "then";
        case ReservedWordKind::Until:
            return "until";
        case ReservedWordKind::While:
            return "while";
        }
        return "";
    }
    static ReservedWordKind getReservedWordKind(util::string_view word) noexcept
    {
        switch(word.size())
        {
        case 1:
            switch(word[0])
            {
            case '!':
                return ReservedWordKind::ExMark;
            case '{':
                return ReservedWordKind::LBrace;
            case '}':
                return ReservedWordKind::RBrace;
            }
            return ReservedWordKind::NotAReservedWord;
        case 2:
            switch(word[0])
            {
            case '[':
                if(word[1] == '[')
                    return ReservedWordKind::DoubleLBracket;
                return ReservedWordKind::NotAReservedWord;
            case ']':
                if(word[1] == ']')
                    return ReservedWordKind::DoubleRBracket;
                return ReservedWordKind::NotAReservedWord;
            case 'd':
                if(word[1] == 'o')
                    return ReservedWordKind::Do;
                return ReservedWordKind::NotAReservedWord;
            case 'f':
                if(word[1] == 'i')
                    return ReservedWordKind::Fi;
                return ReservedWordKind::NotAReservedWord;
            case 'i':
                switch(word[1])
                {
                case 'f':
                    return ReservedWordKind::If;
                case 'n':
                    return ReservedWordKind::In;
                }
                return ReservedWordKind::NotAReservedWord;
            }
            return ReservedWordKind::NotAReservedWord;
        case 3:
            if(word[0] == 'f')
            {
                if(word[1] == 'o')
                {
                    if(word[2] == 'r')
                        return ReservedWordKind::For;
                    return ReservedWordKind::NotAReservedWord;
                }
                return ReservedWordKind::NotAReservedWord;
            }
            return ReservedWordKind::NotAReservedWord;
        case 4:
            switch(word[0])
            {
            case 'c':
                if(word[1] == 'a')
                {
                    if(word[2] == 's')
                    {
                        if(word[3] == 'e')
                            return ReservedWordKind::Case;
                        return ReservedWordKind::NotAReservedWord;
                    }
                    return ReservedWordKind::NotAReservedWord;
                }
                return ReservedWordKind::NotAReservedWord;
            case 'd':
                if(word[1] == 'o')
                {
                    if(word[2] == 'n')
                    {
                        if(word[3] == 'e')
                            return ReservedWordKind::Done;
                        return ReservedWordKind::NotAReservedWord;
                    }
                    return ReservedWordKind::NotAReservedWord;
                }
                return ReservedWordKind::NotAReservedWord;
            case 'e':
                switch(word[1])
                {
                case 'l':
                    switch(word[2])
                    {
                    case 'i':
                        if(word[3] == 'f')
                            return ReservedWordKind::ElIf;
                        return ReservedWordKind::NotAReservedWord;
                    case 's':
                        if(word[3] == 'e')
                            return ReservedWordKind::Else;
                        return ReservedWordKind::NotAReservedWord;
                    }
                    return ReservedWordKind::NotAReservedWord;
                case 's':
                    if(word[2] == 'a')
                    {
                        if(word[3] == 'c')
                            return ReservedWordKind::Esac;
                        return ReservedWordKind::NotAReservedWord;
                    }
                    return ReservedWordKind::NotAReservedWord;
                }
                return ReservedWordKind::NotAReservedWord;
            case 't':
                switch(word[1])
                {
                case 'h':
                    if(word[2] == 'e')
                    {
                        if(word[3] == 'n')
                            return ReservedWordKind::Then;
                        return ReservedWordKind::NotAReservedWord;
                    }
                    return ReservedWordKind::NotAReservedWord;
                case 'i':
                    if(word[2] == 'm')
                    {
                        if(word[3] == 'e')
                            return ReservedWordKind::Time;
                        return ReservedWordKind::NotAReservedWord;
                    }
                    return ReservedWordKind::NotAReservedWord;
                }
                return ReservedWordKind::NotAReservedWord;
            }
            return ReservedWordKind::NotAReservedWord;
        case 5:
            switch(word[0])
            {
            case 'u':
                if(word[1] == 'n')
                {
                    if(word[2] == 't')
                    {
                        if(word[3] == 'i')
                        {
                            if(word[4] == 'l')
                                return ReservedWordKind::Until;
                            return ReservedWordKind::NotAReservedWord;
                        }
                        return ReservedWordKind::NotAReservedWord;
                    }
                    return ReservedWordKind::NotAReservedWord;
                }
                return ReservedWordKind::NotAReservedWord;
            case 'w':
                if(word[1] == 'h')
                {
                    if(word[2] == 'i')
                    {
                        if(word[3] == 'l')
                        {
                            if(word[4] == 'e')
                                return ReservedWordKind::Until;
                            return ReservedWordKind::NotAReservedWord;
                        }
                        return ReservedWordKind::NotAReservedWord;
                    }
                    return ReservedWordKind::NotAReservedWord;
                }
                return ReservedWordKind::NotAReservedWord;
            }
            return ReservedWordKind::NotAReservedWord;
        case 6:
            switch(word[0])
            {
            case 'c':
                if(word[1] == 'o')
                {
                    if(word[2] == 'p')
                    {
                        if(word[3] == 'r')
                        {
                            if(word[4] == 'o')
                            {
                                if(word[5] == 'c')
                                    return ReservedWordKind::Coproc;
                                return ReservedWordKind::NotAReservedWord;
                            }
                            return ReservedWordKind::NotAReservedWord;
                        }
                        return ReservedWordKind::NotAReservedWord;
                    }
                    return ReservedWordKind::NotAReservedWord;
                }
                return ReservedWordKind::NotAReservedWord;
            case 's':
                if(word[1] == 'e')
                {
                    if(word[2] == 'l')
                    {
                        if(word[3] == 'e')
                        {
                            if(word[4] == 'c')
                            {
                                if(word[5] == 't')
                                    return ReservedWordKind::Select;
                                return ReservedWordKind::NotAReservedWord;
                            }
                            return ReservedWordKind::NotAReservedWord;
                        }
                        return ReservedWordKind::NotAReservedWord;
                    }
                    return ReservedWordKind::NotAReservedWord;
                }
                return ReservedWordKind::NotAReservedWord;
            }
            return ReservedWordKind::NotAReservedWord;
        case 8:
            if(word[0] == 'f')
            {
                if(word[1] == 'u')
                {
                    if(word[2] == 'n')
                    {
                        if(word[3] == 'c')
                        {
                            if(word[4] == 't')
                            {
                                if(word[5] == 'i')
                                {
                                    if(word[6] == 'o')
                                    {
                                        if(word[7] == 'n')
                                            return ReservedWordKind::Function;
                                        return ReservedWordKind::NotAReservedWord;
                                    }
                                    return ReservedWordKind::NotAReservedWord;
                                }
                                return ReservedWordKind::NotAReservedWord;
                            }
                            return ReservedWordKind::NotAReservedWord;
                        }
                        return ReservedWordKind::NotAReservedWord;
                    }
                    return ReservedWordKind::NotAReservedWord;
                }
                return ReservedWordKind::NotAReservedWord;
            }
            return ReservedWordKind::NotAReservedWord;
        }
        return ReservedWordKind::NotAReservedWord;
    }

private:
#error finish
};
}
}

#endif /* PARSER_PARSER_H_ */
