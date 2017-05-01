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
    bool parseBlank(input::TextInput::Iterator &textIter)
    {
        if(*textIter == ' ' || *textIter == '\t')
#error finish
    }
#error finish
    static constexpr bool isMetacharacter(int ch) noexcept
    {
        return ch == '|' || ch == '&' || ch == ';' || ch == '(' || ch == ')' || ch == '<'
               || ch == '>' || isBlank(ch) || isNewline(ch);
    }
    static constexpr bool isMetacharacterOrEOF(int ch) noexcept
    {
        return isMetacharacter(ch) || ch == TextInput::eof;
    }
    static bool isNameStartCharacter(int ch) noexcept
    {
        if(ch >= 'a' && ch <= 'z')
            return true;
        if(ch >= 'A' && ch <= 'Z')
            return true;
        return ch == '_';
    }
    static bool isNameContinueCharacter(int ch) noexcept
    {
        if(isNameStartCharacter(ch))
            return true;
        if(ch >= '0' && ch <= '9')
            return true;
        return false;
    }
    static bool isWordStartCharacter(int ch) noexcept
    {
        if(isMetacharacterOrEOF(ch))
            return false;
        switch(ch)
        {
        case '\"':
        case '\'':
        case '!':
        case '$':
        case '`':
        case '\\':
            return false;
        }
        return true;
    }
    static bool isWordContinueCharacter(int ch) noexcept
    {
        if(isWordStartCharacter(ch))
            return true;
        switch(ch)
        {
        case '#':
            return true;
        }
        return false;
    }
    void getAndAddChar()
    {
        tokenValue += static_cast<char>(getChar());
    }
    static bool isCommandTerminator(TokenType tokenType, bool inBackticks) noexcept
    {
        switch(tokenType)
        {
        case TokenType::EndOfFile:
        case TokenType::Newline:
        case TokenType::RParen:
        case TokenType::Semicolon:
            return true;
        case TokenType::Backtick:
            return inBackticks;
        case TokenType::Blanks:
        case TokenType::SingleQuote:
        case TokenType::DollarSingleQuoteStart:
        case TokenType::DollarSingleQuoteEnd:
        case TokenType::UnquotedWordPart:
        case TokenType::QuotedWordPart:
        case TokenType::UnquotedSimpleSubstitution:
        case TokenType::QuotedSimpleSubstitution:
        case TokenType::UnquotedSubstitutionStart:
        case TokenType::QuotedSubstitutionStart:
        case TokenType::UnquotedShellSubstitutionStart:
        case TokenType::QuotedShellSubstitutionStart:
        case TokenType::LParen:
        case TokenType::Equal:
        case TokenType::ExMark:
        case TokenType::LBrace:
        case TokenType::RBrace:
        case TokenType::LBracket:
        case TokenType::RBracket:
        case TokenType::DoubleLBracket:
        case TokenType::DoubleRBracket:
        case TokenType::DoubleLParen:
        case TokenType::DoubleRParen:
        case TokenType::Comment:
        case TokenType::Name:
        case TokenType::Case:
        case TokenType::Coproc:
        case TokenType::Do:
        case TokenType::Done:
        case TokenType::ElIf:
        case TokenType::Else:
        case TokenType::Esac:
        case TokenType::Fi:
        case TokenType::For:
        case TokenType::Function:
        case TokenType::If:
        case TokenType::In:
        case TokenType::Select:
        case TokenType::Time:
        case TokenType::Then:
        case TokenType::Until:
        case TokenType::While:
            return false;
        }
        UNREACHABLE();
        return false;
    }
    static bool isWordTerminator(TokenType tokenType, bool inBackticks) noexcept
    {
        switch(tokenType)
        {
        case TokenType::EndOfFile:
        case TokenType::Newline:
        case TokenType::RParen:
        case TokenType::Semicolon:
        case TokenType::Blanks:
        case TokenType::LParen:
        case TokenType::Comment:
            return true;
        case TokenType::Backtick:
            return inBackticks;
        case TokenType::SingleQuote:
        case TokenType::DollarSingleQuoteStart:
        case TokenType::DollarSingleQuoteEnd:
        case TokenType::UnquotedWordPart:
        case TokenType::QuotedWordPart:
        case TokenType::UnquotedSimpleSubstitution:
        case TokenType::QuotedSimpleSubstitution:
        case TokenType::UnquotedSubstitutionStart:
        case TokenType::QuotedSubstitutionStart:
        case TokenType::UnquotedShellSubstitutionStart:
        case TokenType::QuotedShellSubstitutionStart:
        case TokenType::Equal:
        case TokenType::ExMark:
        case TokenType::LBrace:
        case TokenType::RBrace:
        case TokenType::LBracket:
        case TokenType::RBracket:
        case TokenType::DoubleLBracket:
        case TokenType::DoubleRBracket:
        case TokenType::DoubleLParen:
        case TokenType::DoubleRParen:
        case TokenType::Name:
        case TokenType::Case:
        case TokenType::Coproc:
        case TokenType::Do:
        case TokenType::Done:
        case TokenType::ElIf:
        case TokenType::Else:
        case TokenType::Esac:
        case TokenType::Fi:
        case TokenType::For:
        case TokenType::Function:
        case TokenType::If:
        case TokenType::In:
        case TokenType::Select:
        case TokenType::Time:
        case TokenType::Then:
        case TokenType::Until:
        case TokenType::While:
            return false;
        }
        UNREACHABLE();
        return false;
    }
};

template <bool DebuggingEnabled>
struct ParserTemplateBase
{
protected:
    std::ostream *debugOutput = nullptr;
    explicit ParserTemplateBase(std::ostream *debugOutput) noexcept : debugOutput(debugOutput)
    {
    }
};

template <>
struct ParserTemplateBase<false>
{
protected:
    static constexpr std::ostream *debugOutput = nullptr;
    explicit ParserTemplateBase(std::ostream *debugOutput) noexcept
    {
        assert(debugOutput == nullptr && "debug output is disabled");
    }
};

template <typename ParserActions = ParserActions, bool DebuggingEnabled = false>
class Parser final : private ParserTemplateBase<DebuggingEnabled>, public ParserBase
{
private:
    using ParserTemplateBase<DebuggingEnabled>::debugOutput;
    ParserActions &actions;
    std::size_t parserLevel = 0;

private:
    struct PushLevel final
    {
        Parser *parser;
        explicit PushLevel(Parser *parser) noexcept : parser(parser)
        {
            parser->parserLevel++;
        }
        PushLevel(const PushLevel &) = delete;
        PushLevel &operator=(const PushLevel &) = delete;
        ~PushLevel()
        {
            parser->parserLevel--;
        }
    };

public:
    constexpr Parser(ParserActions &actions,
                     TextInput &textInput,
                     std::ostream *debugOutput = nullptr)
        : ParserTemplateBase<DebuggingEnabled>(debugOutput), ParserBase(textInput), actions(actions)
    {
    }
    ParserActions &getActions() noexcept
    {
        return actions;
    }

private:
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
