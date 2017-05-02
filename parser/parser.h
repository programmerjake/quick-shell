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
#include <cassert>
#include <sstream>
#include <vector>
#include <stdexcept>
#include "../util/compiler_intrinsics.h"
#include "../input/text_input.h"
#include "../input/location.h"
#include "../util/variant.h"

namespace quick_shell
{
namespace parser
{
struct ParseError : public std::runtime_error
{
    input::Location location;
    std::string message;
    static std::string makeWhatMessage(input::Location location, const std::string &message)
    {
        std::ostringstream os;
        os << location << ": " << message;
        return os.str();
    }
    explicit ParseError(input::Location location, std::string message)
        : runtime_error(makeWhatMessage(location, message)),
          location(std::move(location)),
          message(std::move(message))
    {
    }
};

enum class ParseCommandResult
{
    Success,
    NoCommand,
    Quit,
};

class Parser final
{
    Parser(const Parser &) = delete;
    Parser &operator=(const Parser &) = delete;

public:
    input::TextInput &textInput;

public:
    explicit Parser(input::TextInput &textInput) : textInput(textInput)
    {
    }

private:
    static void escapeStringForDebug(std::ostream &os, util::string_view stringIn)
    {
        for(unsigned char ch : stringIn)
        {
            switch(ch)
            {
            case '\'':
            case '\"':
            case '\\':
                os << '\\';
                os << ch;
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
                    os.width(2);
                    os.fill('0');
                    os << std::hex << std::uppercase;
                    os << static_cast<unsigned>(ch & 0xFF);
                    continue;
                }
                os << ch;
                continue;
            }
        }
    }

private:
    struct IteratorCopy final
    {
        input::TextInput::Iterator value;
        explicit IteratorCopy(const input::TextInput::Iterator &value) noexcept : value(value)
        {
        }
        operator input::TextInput::Iterator &() noexcept
        {
            return value;
        }
    };
    static IteratorCopy copy(const input::TextInput::Iterator &value) noexcept
    {
        return IteratorCopy(value);
    }

private:
    union GenerateParseErrorFnArgument final
    {
        void *object;
        void (*function)();
        constexpr GenerateParseErrorFnArgument(void *object = nullptr) noexcept : object(object)
        {
        }
        constexpr GenerateParseErrorFnArgument(void (*function)()) noexcept : function(function)
        {
        }
    };
    typedef void (*GenerateParseErrorFn)(Parser &parser,
                                         input::SimpleLocation location,
                                         GenerateParseErrorFnArgument argument);
    struct ParseResultError final
    {
        GenerateParseErrorFn function;
        input::SimpleLocation location;
        GenerateParseErrorFnArgument argument;
        constexpr ParseResultError() noexcept : function(nullptr), location(), argument()
        {
        }
        constexpr ParseResultError(GenerateParseErrorFn function,
                                   input::SimpleLocation location,
                                   GenerateParseErrorFnArgument argument) noexcept
            : function(function),
              location(location),
              argument(argument)
        {
        }
        void throwError(Parser &parser) const
        {
            function(parser, location, argument);
            UNREACHABLE();
        }
    };
    template <typename T = void>
    struct ParseResult final
    {
        typedef typename std::conditional<std::is_void<T>::value,
                                          util::variant<ParseResultError>,
                                          util::variant<ParseResultError, T>>::type VariantType;
        VariantType value;
        template <typename = typename std::enable_if<std::is_void<T>::value>::type>
        ParseResult() noexcept : value()
        {
        }
        ParseResult(typename std::enable_if<std::is_void<T>::value, T>::type &&value) : value(value)
        {
        }
        ParseResult(const typename std::enable_if<std::is_void<T>::value, T>::type &value)
            : value(value)
        {
        }
        ParseResult(ParseResultError parseResultError) noexcept : value(parseResultError)
        {
        }
        explicit ParseResult(
            GenerateParseErrorFn function,
            input::SimpleLocation location = input::SimpleLocation(),
            GenerateParseErrorFnArgument argument = GenerateParseErrorFnArgument()) noexcept
            : value(ParseResultError(function, location, argument))
        {
        }
        explicit ParseResult(void (*function)(Parser &parser, input::SimpleLocation location),
                             input::SimpleLocation location) noexcept
            : value(ParseResultError(
                  [](Parser &parser,
                     input::SimpleLocation location,
                     GenerateParseErrorFnArgument argument)
                  {
                      reinterpret_cast<void (*)(Parser &, input::SimpleLocation location)>(
                          argument.function)(parser, location);
                      UNREACHABLE();
                  },
                  location,
                  reinterpret_cast<void (*)()>(function)))
        {
        }
        explicit ParseResult(void (*function)(Parser &parser, input::Location location),
                             input::SimpleLocation location) noexcept
            : value(ParseResultError(
                  [](Parser &parser,
                     input::SimpleLocation location,
                     GenerateParseErrorFnArgument argument)
                  {
                      reinterpret_cast<void (*)(Parser &, input::Location location)>(
                          argument.function)(parser, input::Location(location, parser.textInput));
                      UNREACHABLE();
                  },
                  location,
                  reinterpret_cast<void (*)()>(function)))
        {
        }
        explicit ParseResult(void (*function)(input::Location location),
                             input::SimpleLocation location) noexcept
            : value(ParseResultError(
                  [](Parser &parser,
                     input::SimpleLocation location,
                     GenerateParseErrorFnArgument argument)
                  {
                      reinterpret_cast<void (*)(input::Location location)>(argument.function)(
                          input::Location(location, parser.textInput));
                      UNREACHABLE();
                  },
                  location,
                  reinterpret_cast<void (*)()>(function)))
        {
        }
        explicit ParseResult(void (*function)(input::SimpleLocation location),
                             input::SimpleLocation location) noexcept
            : value(ParseResultError(
                  [](Parser &parser,
                     input::SimpleLocation location,
                     GenerateParseErrorFnArgument argument)
                  {
                      reinterpret_cast<void (*)(input::Location location)>(argument.function)(
                          location);
                      UNREACHABLE();
                  },
                  location,
                  reinterpret_cast<void (*)()>(function)))
        {
        }
        explicit ParseResult(void (*function)(input::SimpleLocation location),
                             input::SimpleLocation location) noexcept
            : value(ParseResultError(
                  [](Parser &parser,
                     input::SimpleLocation location,
                     GenerateParseErrorFnArgument argument)
                  {
                      reinterpret_cast<void (*)(input::Location location)>(argument.function)(
                          location);
                      UNREACHABLE();
                  },
                  location,
                  reinterpret_cast<void (*)()>(function)))
        {
        }
        constexpr bool isError() const noexcept
        {
            return value.is<ParseResultError>();
        }
        constexpr bool isSuccess() const noexcept
        {
            return !isError();
        }
        void throwError(Parser &parser) const
        {
            assert(isError());
            value.get<ParseResultError>().throwError(parser);
        }
        constexpr explicit operator bool() const noexcept
        {
            return isSuccess();
        }
    };
    template <typename T = void, typename... Args>
    ParseResult<T> parserError(Args &&... args) noexcept(
        noexcept(ParseResult<T>(std::forward<Args>(args)...)))
    {
        return ParseResult<T>(std::forward<Args>(args)...);
    }
    template <std::size_t N>
    ParseResultError parserErrorStaticString(const char(&message)[N],
                                             input::SimpleLocation location) noexcept
    {
        return ParseResultError(
            [](Parser &parser,
               input::SimpleLocation location,
               GenerateParseErrorFnArgument argument)
            {
                throw ParseError(input::Location(location, parser.textInput),
                                 static_cast<const char *>(argument.object));
            },
            location,
            static_cast<void *>(const_cast<char *>(message)));
    }
    template <std::size_t N>
    ParseResultError parserErrorStaticString(const char(&message)[N],
                                             const input::TextInput::Iterator &iter) noexcept
    {
        return parserErrorStaticString(message, input::SimpleLocation(iter.getIndex()));
    }
    ParseResult<void> parserSuccess() noexcept
    {
        return ParseResult<void>();
    }
    template <typename T>
    ParseResult<T> parserSuccess(const T &v)
    {
        return ParseResult<T>(v);
    }
    template <typename T>
    ParseResult<T> parserSuccess(T &&v)
    {
        return ParseResult<T>(std::move(v));
    }

private:
    ParseResult<> parseBlank(input::TextInput::Iterator &textIter)
    {
        switch(*textIter)
        {
        case ' ':
        case '\t':
            ++textIter;
            return parserSuccess();
        default:
            return parserErrorStaticString("missing blank", textIter);
        }
    }
    ParseResult<> parseMetacharacter(input::TextInput::Iterator &textIter)
    {
        switch(*textIter)
        {
        case '|':
        case '&':
        case ';':
        case '(':
        case ')':
        case '<':
        case '>':
            ++textIter;
            return parserSuccess();
        default:
        {
            auto textIter2 = textIter;
            auto retval = parseBlank(textIter2);
            if(retval)
            {
                textIter = textIter2;
                return retval;
            }
            textIter2 = textIter;
            auto retval = parseBlank(textIter2);
            if(retval)
            {
                textIter = textIter2;
                return retval;
            }
            return parserErrorStaticString("missing metacharacter", textIter);
        }
        }
    }
    ParseResult<> parseMetacharacterOrEOF(input::TextInput::Iterator &textIter)
    {
        if(*textIter == input::eof)
        {
            ++textIter;
            return parserSuccess();
        }
        return parseMetacharacter(textIter);
    }
    ParseResult<> parseNameStartCharacter(input::TextInput::Iterator &textIter)
    {
        int ch = *textIter;
        if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_')
        {
            ++textIter;
            return parserSuccess();
        }
        return parserErrorStaticString("missing name start character", textIter);
    }
    ParseResult<> parseNameContinueCharacter(input::TextInput::Iterator &textIter)
    {
        int ch = *textIter;
        if(parseNameStartCharacter(copy(textIter)) || (ch >= '0' && ch <= '9'))
        {
            ++textIter;
            return parserSuccess();
        }
        return parserErrorStaticString("missing name continue character", textIter);
    }
    ParseResult<> parseWordStartCharacter(input::TextInput::Iterator &textIter)
    {
        switch(*textIter)
        {
        case '\"':
        case '\'':
        case '!':
        case '$':
        case '`':
        case '\\':
        case '#':
            break;
        default:
            if(!parseMetacharacterOrEOF(copy(textIter)))
            {
                ++textIter;
                return parserSuccess();
            }
        }
        return parserErrorStaticString("missing word start character", textIter);
    }
    ParseResult<> parseWordContinueCharacter(input::TextInput::Iterator &textIter)
    {
        if(parseWordStartCharacter(copy(textIter)) || *textIter == '#')
        {
            ++textIter;
            return parserSuccess();
        }
        return parserErrorStaticString("missing word continue character", textIter);
    }

private:
#error finish
    void tokenizeReservedWord()
    {
        auto oldTokenType = tokenType;
        tokenType = tokenizeReservedWord(tokenType, tokenValue);
        if(tokenType != oldTokenType && debugOutput)
        {
            *debugOutput << getTokenDebugString() << std::endl;
        }
    }
    static TokenType tokenizeReservedWord(TokenType type, util::string_view value) noexcept
    {
        assert(type == TokenType::UnquotedWordPart || type == TokenType::Name);
        struct ReservedWord
        {
            TokenType tokenType;
            util::string_view word;
        };
        static const ReservedWord reservedWords[] = {
            {TokenType::ExMark, "!"},
            {TokenType::LBrace, "{"},
            {TokenType::RBrace, "}"},
            {TokenType::DoubleLBracket, "[["},
            {TokenType::DoubleRBracket, "]]"},
            {TokenType::Case, "case"},
            {TokenType::Coproc, "coproc"},
            {TokenType::Do, "do"},
            {TokenType::Done, "done"},
            {TokenType::ElIf, "elif"},
            {TokenType::Else, "else"},
            {TokenType::Esac, "esac"},
            {TokenType::Fi, "fi"},
            {TokenType::For, "for"},
            {TokenType::Function, "function"},
            {TokenType::If, "if"},
            {TokenType::In, "in"},
            {TokenType::Select, "select"},
            {TokenType::Time, "time"},
            {TokenType::Then, "then"},
            {TokenType::Until, "until"},
            {TokenType::While, "while"},
        };
        for(auto &reservedWord : reservedWords)
        {
            if(reservedWord.word == value)
            {
                type = reservedWord.tokenType;
                break;
            }
        }
        return type;
    }
    void reparseToken(TokenizerMode mode, bool outputAndSkipComments = true)
    {
        rewind();
        parseNextToken(mode, outputAndSkipComments);
    }
    void parseNextToken(TokenizerMode mode, bool outputAndSkipComments = true)
    {
        for(;;)
        {
            discardRewindBuffer();
            peekChar(); // to set location
            isInitialToken = false;
            tokenLocation = charLocation;
            tokenType = TokenType::EndOfFile;
            tokenValue.clear();
            if(peekChar() == TextInput::eof)
            {
                tokenType = TokenType::EndOfFile;
                getChar();
            }
            else if(isNewline(peekChar()))
            {
                tokenType = TokenType::Newline;
                getAndAddChar();
            }
            else
            {
                switch(mode)
                {
                case TokenizerMode::Command:
                case TokenizerMode::CommandWithNames:
                    switch(peekChar())
                    {
                    case '#':
                        tokenType = TokenType::Comment;
                        getAndAddChar();
                        while(!isNewlineOrEOF(peekChar()))
                        {
                            getAndAddChar();
                        }
                        break;
                    case '(':
                        tokenType = TokenType::LParen;
                        getAndAddChar();
                        if(peekChar() == '(')
                        {
                            tokenType = TokenType::DoubleLParen;
                            getAndAddChar();
                        }
                        break;
                    case ')':
                        tokenType = TokenType::RParen;
                        getAndAddChar();
                        if(peekChar() == ')')
                        {
                            tokenType = TokenType::DoubleRParen;
                            getAndAddChar();
                        }
                        break;
                    case '=':
                        tokenType = TokenType::Equal;
                        getAndAddChar();
                        break;
                    case ';':
                        tokenType = TokenType::Semicolon;
                        getAndAddChar();
                        break;
                    case '`':
                        tokenType = TokenType::Backtick;
                        getAndAddChar();
                        break;
                    default:
                        if(isBlank(peekChar()))
                        {
                            tokenType = TokenType::Blanks;
                            do
                            {
                                getAndAddChar();
                            } while(isBlank(peekChar()));
                        }
                        else if(isWordStartCharacter(peekChar()))
                        {
                            tokenType = TokenType::UnquotedWordPart;
                            if(mode == TokenizerMode::CommandWithNames
                               && isNameStartCharacter(peekChar()))
                                tokenType = TokenType::Name;
                            do
                            {
                                if(tokenType == TokenType::Name)
                                {
                                    if(peekChar() == '[' || peekChar() == '=')
                                        break;
                                    if(!isNameContinueCharacter(peekChar()))
                                        tokenType = TokenType::UnquotedWordPart;
                                }
                                getAndAddChar();
                            } while(isWordContinueCharacter(peekChar()));
                        }
                        else
                        {
                            UNIMPLEMENTED();
                        }
                    }
                    break;
                case TokenizerMode::DoubleQuotes:
                    UNIMPLEMENTED();
                    break;
                case TokenizerMode::VariableSubstitution:
                    UNIMPLEMENTED();
                    break;
                case TokenizerMode::Arithmetic:
                    UNIMPLEMENTED();
                    break;
                case TokenizerMode::Condition:
                    UNIMPLEMENTED();
                    break;
                case TokenizerMode::RegEx:
                    UNIMPLEMENTED();
                    break;
                case TokenizerMode::RegExCharacterClass:
                    UNIMPLEMENTED();
                    break;
                }
            }
            if(debugOutput)
            {
                *debugOutput << getTokenDebugString() << std::endl;
            }
            if(outputAndSkipComments && tokenType == TokenType::Comment)
            {
                outputToken();
                continue;
            }
            break;
        }
    }
    void outputToken() const
    {
        if(!isInitialToken)
            outputToken(tokenLocation, tokenType, tokenValue);
    }
    void outputToken(Location location, TokenType type, const std::string &value) const
    {
        actions.handleToken(parserLevel, location, type, value);
    }
    void outputAndNextToken(TokenizerMode tokenizerMode, bool outputAndSkipComments = true)
    {
        outputToken();
        parseNextToken(tokenizerMode, outputAndSkipComments);
    }

private:
    void parseUnsplitWord(bool inBackticks)
    {
        PushLevel pushLevel(this);
        reparseToken(TokenizerMode::Command, false);
        auto initialTokenLocation = tokenLocation;
        auto initialTokenType = tokenType;
        auto initialTokenValue = tokenValue;
        outputAndNextToken(TokenizerMode::Command, false);
        parseUnsplitWordContinuation(
            inBackticks, initialTokenLocation, initialTokenType, initialTokenValue);
    }
    void parseUnsplitWordContinuation(bool inBackticks,
                                      Location initialTokenLocation,
                                      TokenType initialTokenType,
                                      const std::string &initialTokenValue)
    {
        PushLevel pushLevel(this);
        if(isWordTerminator(tokenType, inBackticks))
            return;
        reparseToken(TokenizerMode::Command, false);
        while(true)
        {
            UNIMPLEMENTED();
        }
    }
    void parseSimpleVariableAssignment(std::string &&variableName,
                                       Location variableNameLocation,
                                       bool inBackticks)
    {
        assert(tokenType == TokenType::Equal);
        outputAndNextToken(TokenizerMode::Command);
        UNIMPLEMENTED();
    }
    bool parseSimpleCommand(bool inBackticks)
    {
        PushLevel pushLevel(this);
        bool parsedAnything = false;
        reparseToken(TokenizerMode::CommandWithNames);
        for(;;)
        {
            if(tokenType == TokenType::Blanks)
            {
                outputAndNextToken(TokenizerMode::CommandWithNames);
                continue;
            }
            else if(tokenType == TokenType::Name)
            {
                tokenizeReservedWord();
                auto initialTokenType = tokenType;
                auto initialTokenValue = tokenValue;
                auto initialTokenLocation = tokenLocation;
                parseNextToken(TokenizerMode::CommandWithNames);
                if(tokenType == TokenType::Equal) // variable assignment
                {
                    initialTokenType = TokenType::Name;
                    outputToken(initialTokenLocation, initialTokenType, initialTokenValue);
                    parseSimpleVariableAssignment(
                        std::move(initialTokenValue), initialTokenLocation, inBackticks);
                    parsedAnything = true;
                    continue;
                }
                else if(tokenType == TokenType::LBracket) // variable assignment
                {
                    UNIMPLEMENTED();
                    continue;
                }
                else if(tokenType == TokenType::Blanks
                        || isCommandTerminator(tokenType, inBackticks))
                {
                }
                else
                {
                    outputToken(initialTokenLocation, initialTokenType, initialTokenValue);
                    UNIMPLEMENTED();
                }
            }
            else if(isCommandTerminator(tokenType, inBackticks))
            {
                return parsedAnything;
            }
            else
            {
                UNIMPLEMENTED();
            }
        }
    }

public:
    ParseCommandResult parseCommand()
    {
        PushLevel pushLevel(this);
        parseNextToken(TokenizerMode::CommandWithNames);
        while(tokenType == TokenType::Blanks)
            outputAndNextToken(TokenizerMode::CommandWithNames);
        if(tokenType == TokenType::Newline)
        {
            outputToken();
            return ParseCommandResult::NoCommand;
        }
        if(tokenType == TokenType::EndOfFile)
        {
            outputToken();
            return ParseCommandResult::Quit;
        }
#warning finish
        return parseSimpleCommand(false) ? ParseCommandResult::Success :
                                           ParseCommandResult::NoCommand;
    }
};

struct ParserActions
{
    virtual ~ParserActions() = default;
    virtual void handleToken(std::size_t parserLevel,
                             Location location,
                             ParserBase::TokenType tokenType,
                             const std::string &tokenValue) = 0;
};
}
}

#endif /* PARSER_PARSER_H_ */
