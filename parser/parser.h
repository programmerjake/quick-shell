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
#include <type_traits>
#include <limits>
#include "../util/compiler_intrinsics.h"
#include "../input/text_input.h"
#include "../input/location.h"
#include "../util/variant.h"
#include "../util/string_view.h"
#include "../ast/blank.h"
#include "../ast/word.h"
#include "../ast/word_part.h"
#include "../util/arena.h"
#include "../util/unicode.h"

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

struct ParserDialect final
{
    input::TextInputStyle textInputStyle;
    bool allowDollarSingleQuoteStrings;
    bool duplicateDollarSingleQuoteStringBashParsingFlaws;
    constexpr ParserDialect() noexcept : ParserDialect(QuickShellDialectTag{})
    {
    }

private:
    struct BashDialectTag final
    {
    };
    struct PosixDialectTag final
    {
    };
    struct QuickShellDialectTag final
    {
    };
    constexpr explicit ParserDialect(BashDialectTag) noexcept
        : textInputStyle(8, false, false, true),
          allowDollarSingleQuoteStrings(true),
          duplicateDollarSingleQuoteStringBashParsingFlaws(true)
    {
    }
    constexpr explicit ParserDialect(PosixDialectTag) noexcept
        : textInputStyle(8, false, false, true),
          allowDollarSingleQuoteStrings(false),
          duplicateDollarSingleQuoteStringBashParsingFlaws(false)
    {
    }
    constexpr explicit ParserDialect(QuickShellDialectTag) noexcept
        : textInputStyle(8, true, true, true),
          allowDollarSingleQuoteStrings(true),
          duplicateDollarSingleQuoteStringBashParsingFlaws(false)
    {
    }

public:
    /** bash compatible; includes duplicating bash bugs */
    constexpr static ParserDialect getBashDialect() noexcept
    {
        return ParserDialect(BashDialectTag{});
    }
    /** mostly bash compatible; enables Quick Shell's extensions */
    constexpr static ParserDialect getQuickShellDialect() noexcept
    {
        return ParserDialect(QuickShellDialectTag{});
    }
    /** Posix shell */
    static ParserDialect getPosixDialect() noexcept
    {
        return ParserDialect(PosixDialectTag{});
    }
};

class Parser final
{
    Parser(const Parser &) = delete;
    Parser &operator=(const Parser &) = delete;

private:
    input::TextInput &textInput;
    util::Arena &arena;
    const ParserDialect dialect;

public:
    explicit Parser(input::TextInput &textInput,
                    util::Arena &arena,
                    const ParserDialect &dialect = ParserDialect::getQuickShellDialect())
        : textInput(textInput), arena(arena), dialect(dialect)
    {
        textInput.setInputStyle(dialect.textInputStyle);
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
    template <typename IteratorType>
    struct IteratorCopy final
    {
        IteratorType value;
        explicit IteratorCopy(const IteratorType &value) : value(value)
        {
        }
        operator IteratorType &() noexcept
        {
            return value;
        }
    };
    template <typename IteratorType>
    static IteratorCopy<IteratorType> copy(const IteratorType &value)
    {
        return IteratorCopy<IteratorType>(value);
    }

private:
    union GenerateParseErrorFnArgument final
    {
        void *object;
        void (*function)();
        std::size_t integer;
        constexpr GenerateParseErrorFnArgument(void *object = nullptr) noexcept : object(object)
        {
        }
        constexpr GenerateParseErrorFnArgument(void (*function)()) noexcept : function(function)
        {
        }
        constexpr GenerateParseErrorFnArgument(std::size_t integer) noexcept : integer(integer)
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
        template <typename T2 = T,
                  typename Bool = typename std::enable_if<std::is_void<T2>::value, bool>::type>
        ParseResult() noexcept(Bool(true))
            : value()
        {
        }
        template <typename T2 = T>
        ParseResult(typename std::enable_if<!std::is_void<T2>::value, T2>::type &&value)
            : value(value)
        {
        }
        template <typename T2 = T>
        ParseResult(const typename std::enable_if<!std::is_void<T2>::value, T2>::type &value)
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
                      reinterpret_cast<void (*)(input::SimpleLocation location)>(argument.function)(
                          location);
                      UNREACHABLE();
                  },
                  location,
                  reinterpret_cast<void (*)()>(function)))
        {
        }
        bool isError() const noexcept
        {
            return value.template is<ParseResultError>();
        }
        bool isSuccess() const noexcept
        {
            return !isError();
        }
        void throwError(Parser &parser) const
        {
            assert(isError());
            value.template get<ParseResultError>().throwError(parser);
        }
        explicit operator bool() const noexcept
        {
            return isSuccess();
        }
        template <typename T2 = T>
        typename std::enable_if<std::is_same<T, T2>::value && !std::is_void<T2>::value, T2>::type &
            get() noexcept
        {
            return value.template get<T2>();
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
        return parserErrorStaticString(message, iter.getLocation());
    }
    template <std::size_t N>
    ParseResultError parserErrorStaticString(
        const char(&message)[N], const input::LineContinuationRemovingIterator &iter) noexcept
    {
        return parserErrorStaticString(message, iter.getLocation());
    }
    ParseResult<void> parserSuccess() noexcept
    {
        return ParseResult<void>();
    }
    template <typename T>
    ParseResult<typename std::decay<T>::type> parserSuccess(T &&v)
    {
        return ParseResult<typename std::decay<T>::type>(std::forward<T>(v));
    }

private:
    ParseResult<> parseNewLine(input::LineContinuationRemovingIterator &textIter)
    {
        auto baseTextIter = textIter.getBaseIterator();
        auto result = parseNewLine(baseTextIter);
        if(result)
            textIter = input::LineContinuationRemovingIterator(baseTextIter);
        return result;
    }
    ParseResult<> parseNewLine(input::TextInput::Iterator &textIter)
    {
        if(*textIter == '\r')
        {
            ++textIter;
            if(dialect.textInputStyle.allowCRLFAsNewLine && *textIter == '\n')
            {
                ++textIter;
                return parserSuccess();
            }
            if(dialect.textInputStyle.allowCRAsNewLine)
                return parserSuccess();
        }
        if(dialect.textInputStyle.allowLFAsNewLine && *textIter == '\n')
        {
            ++textIter;
            return parserSuccess();
        }
        return parserErrorStaticString("missing newline", textIter);
    }
    ParseResult<> parseBlank(input::LineContinuationRemovingIterator &textIter)
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
    ParseResult<> parseMetacharacter(input::LineContinuationRemovingIterator &textIter)
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
            auto retval = parseNewLine(textIter2);
            if(retval)
            {
                textIter = textIter2;
                return retval;
            }
            textIter2 = textIter;
            retval = parseBlank(textIter2);
            if(retval)
            {
                textIter = textIter2;
                return retval;
            }
            return parserErrorStaticString("missing metacharacter", textIter);
        }
        }
    }
    ParseResult<> parseMetacharacterOrEOF(input::LineContinuationRemovingIterator &textIter)
    {
        if(*textIter == input::eof)
        {
            ++textIter;
            return parserSuccess();
        }
        return parseMetacharacter(textIter);
    }
    ParseResult<> parseNameStartCharacter(input::LineContinuationRemovingIterator &textIter)
    {
        int ch = *textIter;
        if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_')
        {
            ++textIter;
            return parserSuccess();
        }
        return parserErrorStaticString("missing name start character", textIter);
    }
    ParseResult<> parseNameContinueCharacter(input::LineContinuationRemovingIterator &textIter)
    {
        int ch = *textIter;
        if(parseNameStartCharacter(copy(textIter)) || (ch >= '0' && ch <= '9'))
        {
            ++textIter;
            return parserSuccess();
        }
        return parserErrorStaticString("missing name continue character", textIter);
    }
    ParseResult<> parseSimpleWordStartCharacter(input::LineContinuationRemovingIterator &textIter)
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
        return parserErrorStaticString("missing unquoted word start character", textIter);
    }
    ParseResult<> parseSimpleWordContinueCharacter(
        input::LineContinuationRemovingIterator &textIter)
    {
        if(parseSimpleWordStartCharacter(copy(textIter)) || *textIter == '#')
        {
            ++textIter;
            return parserSuccess();
        }
        return parserErrorStaticString("missing unquoted word continue character", textIter);
    }
    ParseResult<> parseWordStartCharacter(input::LineContinuationRemovingIterator &textIter,
                                          bool isInsideBackquotes)
    {
        switch(*textIter)
        {
        case '\"':
        case '\'':
        case '$':
        case '!':
        case '\\':
            ++textIter;
            return parserSuccess();
        case '`':
            if(isInsideBackquotes)
                break;
            ++textIter;
            return parserSuccess();
        default:
        {
            auto textIter2 = textIter;
            auto retval = parseSimpleWordStartCharacter(textIter2);
            if(retval)
            {
                textIter = textIter2;
                return retval;
            }
            break;
        }
        }
        return parserErrorStaticString("missing word start character", textIter);
    }
    ParseResult<> parseUnquotedWordEndCharacter(input::LineContinuationRemovingIterator &textIter,
                                                bool isInsideBackquotes)
    {
        switch(*textIter)
        {
        case '`':
            if(isInsideBackquotes)
            {
                ++textIter;
                return parserSuccess();
            }
            break;
        default:
        {
            auto textIter2 = textIter;
            auto retval = parseMetacharacterOrEOF(textIter2);
            if(retval)
            {
                textIter = textIter2;
                return retval;
            }
            break;
        }
        }
        return parserErrorStaticString("missing unquoted word end character", textIter);
    }
    template <typename IteratorType>
    ParseResult<int> parseDigit(IteratorType &textIter, std::size_t base = 10)
    {
        assert(base >= 2 && base <= 36);
        int ch = *textIter;
        int value = -1;
        if(ch >= '0' && ch <= '9')
            value = ch - '0';
        else if(ch >= 'a' && ch <= 'z')
            value = ch - 'a' + 0xA;
        else if(ch >= 'A' && ch <= 'Z')
            value = ch - 'A' + 0xA;
        if(value >= static_cast<int>(base))
            value = -1;
        if(value != -1)
        {
            ++textIter;
            return parserSuccess(value);
        }
        if(base == 10)
            return parserErrorStaticString("missing decimal digit", textIter);
        if(base == 16)
            return parserErrorStaticString("missing hexadecimal digit", textIter);
        if(base == 8)
            return parserErrorStaticString("missing octal digit", textIter);
        if(base == 2)
            return parserErrorStaticString("missing binary digit", textIter);
        return parserError<int>(
            [](Parser &parser,
               input::SimpleLocation location,
               GenerateParseErrorFnArgument argument)
            {
                std::ostringstream ss;
                ss << "missing base-" << argument.integer << " digit";
                throw ParseError(input::Location(location, parser.textInput), ss.str());
            },
            textIter.getLocation(),
            base);
    }
    template <typename NumberType = unsigned long, typename IteratorType>
    ParseResult<NumberType> parseSimpleNumber(IteratorType &textIter,
                                              std::size_t base,
                                              std::size_t minDigitCount,
                                              std::size_t maxDigitCount)
    {
        assert(base >= 2 && base <= 36);
        constexpr NumberType maxValue = std::numeric_limits<NumberType>::max();
        const NumberType maxValueOverBase = maxValue / base;
        const NumberType maxValueModBase = maxValue % base;
        NumberType retval = 0;
        std::size_t digitCount = 0;
        auto startLocation = textIter.getLocation();
        while(digitCount < maxDigitCount)
        {
            auto textIter2 = textIter;
            auto digitResult = parseDigit(textIter2, base);
            if(!digitResult)
            {
                if(digitCount >= minDigitCount)
                    break;
                return parserError<NumberType>(digitResult.value.template get<ParseResultError>());
            }
            textIter = textIter2;
            digitCount++;
            int digitValue = digitResult.get();
            if(retval > maxValueOverBase
               || (retval == maxValueOverBase
                   && static_cast<unsigned>(digitValue) > maxValueModBase))
                return parserErrorStaticString("number too big", startLocation);
            retval *= base;
            retval += digitValue;
        }
        return parserSuccess(retval);
    }
    ParseResult<util::ArenaPtr<ast::Word>> parseWord(
        input::LineContinuationRemovingIterator &textIter,
        bool isInsideBackquotes,
        bool checkForVariableAssignment,
        bool checkForReservedWords)
    {
        auto wordStartLocation = textIter.getLocation();
        if(!parseWordStartCharacter(copy(textIter), isInsideBackquotes))
            return parserErrorStaticString("missing word", textIter);
        std::vector<util::ArenaPtr<ast::WordPart>> wordParts;
        while(!parseUnquotedWordEndCharacter(copy(textIter), isInsideBackquotes))
        {
            if(parseSimpleWordStartCharacter(copy(textIter)))
            {
                auto wordPartStartLocation = textIter.getLocation();
                if(checkForVariableAssignment && !parseNameStartCharacter(copy(textIter)))
                    checkForVariableAssignment = false;
                for(;;)
                {
                    if(!parseSimpleWordContinueCharacter(copy(textIter)))
                    {
                        wordParts.push_back(
                            arena.allocate<ast::TextWordPart<ast::WordPart::QuoteKind::Unquoted>>(
                                input::LocationSpan(wordPartStartLocation,
                                                    textIter.getLocation())));
                        checkForVariableAssignment = false;
                        break;
                    }
                    if(checkForVariableAssignment)
                    {
                        if(*textIter == '=')
                        {
                            wordParts.push_back(arena.allocate<ast::AssignmentVariableNameWordPart>(
                                input::LocationSpan(wordPartStartLocation,
                                                    textIter.getLocation())));
                            auto equalSignStartLocation = textIter.getLocation();
                            ++textIter;
                            wordParts.push_back(arena.allocate<ast::AssignmentEqualSignWordPart>(
                                input::LocationSpan(equalSignStartLocation,
                                                    textIter.getLocation())));
                            checkForVariableAssignment = false;
                            break;
                        }
                        else if(*textIter == '[')
                        {
                            UNIMPLEMENTED();
                        }
                        else if(!parseNameContinueCharacter(copy(textIter)))
                            checkForVariableAssignment = false;
                    }
                    ++textIter;
                }
            }
            else if(*textIter == '\\')
            {
                auto escapeStartLocation = textIter.getLocation();
                auto escapeStartTextIter = textIter;
                ++textIter;
                if(*textIter != input::eof)
                {
                    char value = *textIter;
                    ++textIter;
                    wordParts.push_back(
                        arena.allocate<ast::SimpleEscapeSequenceWordPart<ast::WordPart::QuoteKind::
                                                                             Unquoted>>(
                            input::LocationSpan(escapeStartLocation, textIter.getLocation()),
                            value));
                }
                else
                {
                    textIter = escapeStartTextIter;
                    break;
                }
            }
            else if(*textIter == '\'')
            {
                auto openingQuoteStartLocation = textIter.getLocation();
                auto baseTextIter = textIter.getBaseIterator();
                ++baseTextIter;
                wordParts.push_back(
                    arena.allocate<ast::QuoteWordPart<true, ast::WordPart::QuoteKind::SingleQuote>>(
                        input::LocationSpan(openingQuoteStartLocation,
                                            baseTextIter.getLocation())));
                auto quotedTextStartLocation = baseTextIter.getLocation();
                while(*baseTextIter != '\'')
                {
                    if(*baseTextIter == input::eof)
                        return parserErrorStaticString("missing closing \'",
                                                       quotedTextStartLocation);
                    ++baseTextIter;
                }
                wordParts.push_back(
                    arena.allocate<ast::TextWordPart<ast::WordPart::QuoteKind::SingleQuote>>(
                        input::LocationSpan(quotedTextStartLocation, baseTextIter.getLocation())));
                auto closingQuoteStartLocation = baseTextIter.getLocation();
                textIter = input::LineContinuationRemovingIterator(baseTextIter);
                ++textIter;
                wordParts.push_back(
                    arena
                        .allocate<ast::QuoteWordPart<false, ast::WordPart::QuoteKind::SingleQuote>>(
                            input::LocationSpan(closingQuoteStartLocation,
                                                textIter.getLocation())));
            }
            else if(*textIter == '$')
            {
                auto dollarSignLocation = textIter.getLocation();
                ++textIter;
                if(dialect.allowDollarSingleQuoteStrings && *textIter == '\'')
                {
                    typedef ast::TextWordPart<ast::WordPart::QuoteKind::
                                                  EscapeInterpretingSingleQuote> TextWordPartType;
                    typedef ast::SimpleEscapeSequenceWordPart<ast::WordPart::QuoteKind::
                                                                  EscapeInterpretingSingleQuote>
                        SimpleEscapeSequenceWordPartType;
                    typedef ast::HexEscapeSequenceWordPart<ast::WordPart::QuoteKind::
                                                               EscapeInterpretingSingleQuote>
                        HexEscapeSequenceWordPartType;
                    typedef ast::OctalEscapeSequenceWordPart<ast::WordPart::QuoteKind::
                                                                 EscapeInterpretingSingleQuote>
                        OctalEscapeSequenceWordPartType;
                    typedef ast::UnicodeEscapeSequenceWordPart<ast::WordPart::QuoteKind::
                                                                   EscapeInterpretingSingleQuote>
                        UnicodeEscapeSequenceWordPartType;
                    typedef ast::BashBugEscapeSequenceWordPart<ast::WordPart::QuoteKind::
                                                                   EscapeInterpretingSingleQuote>
                        BashBugEscapeSequenceWordPartType;
                    auto baseTextIter = textIter.getBaseIterator();
                    ++baseTextIter;
                    wordParts.push_back(
                        arena.allocate<ast::QuoteWordPart<true,
                                                          ast::WordPart::QuoteKind::
                                                              EscapeInterpretingSingleQuote>>(
                            input::LocationSpan(dollarSignLocation, baseTextIter.getLocation())));
                    auto quotedTextStartLocation = baseTextIter.getLocation();
                    auto wordPartStartLocation = quotedTextStartLocation;
                    while(*baseTextIter != '\'')
                    {
                        if(*baseTextIter == input::eof)
                            return parserErrorStaticString("missing closing \'",
                                                           quotedTextStartLocation);
                        if(*baseTextIter == '\\')
                        {
                            if(wordPartStartLocation != baseTextIter.getLocation())
                                wordParts.push_back(
                                    arena.allocate<TextWordPartType>(input::LocationSpan(
                                        wordPartStartLocation, baseTextIter.getLocation())));
                            wordPartStartLocation = baseTextIter.getLocation();
                            ++baseTextIter;
                            switch(*baseTextIter)
                            {
                            case input::eof:
                                return parserErrorStaticString("missing closing \'",
                                                               quotedTextStartLocation);
                            case 'a':
                            {
                                ++baseTextIter;
                                auto locationSpan = input::LocationSpan(wordPartStartLocation,
                                                                        baseTextIter.getLocation());
                                wordParts.push_back(
                                    arena.allocate<SimpleEscapeSequenceWordPartType>(locationSpan,
                                                                                     '\a'));
                                wordPartStartLocation = baseTextIter.getLocation();
                                break;
                            }
                            case 'b':
                            {
                                ++baseTextIter;
                                auto locationSpan = input::LocationSpan(wordPartStartLocation,
                                                                        baseTextIter.getLocation());
                                wordParts.push_back(
                                    arena.allocate<SimpleEscapeSequenceWordPartType>(locationSpan,
                                                                                     '\b'));
                                wordPartStartLocation = baseTextIter.getLocation();
                                break;
                            }
                            case 'e':
                            case 'E':
                            {
                                ++baseTextIter;
                                auto locationSpan = input::LocationSpan(wordPartStartLocation,
                                                                        baseTextIter.getLocation());
                                wordParts.push_back(
                                    arena.allocate<SimpleEscapeSequenceWordPartType>(locationSpan,
                                                                                     '\x1B'));
                                wordPartStartLocation = baseTextIter.getLocation();
                                break;
                            }
                            case 'f':
                            {
                                ++baseTextIter;
                                auto locationSpan = input::LocationSpan(wordPartStartLocation,
                                                                        baseTextIter.getLocation());
                                wordParts.push_back(
                                    arena.allocate<SimpleEscapeSequenceWordPartType>(locationSpan,
                                                                                     '\f'));
                                wordPartStartLocation = baseTextIter.getLocation();
                                break;
                            }
                            case 'n':
                            {
                                ++baseTextIter;
                                auto locationSpan = input::LocationSpan(wordPartStartLocation,
                                                                        baseTextIter.getLocation());
                                wordParts.push_back(
                                    arena.allocate<SimpleEscapeSequenceWordPartType>(locationSpan,
                                                                                     '\n'));
                                wordPartStartLocation = baseTextIter.getLocation();
                                break;
                            }
                            case 'r':
                            {
                                ++baseTextIter;
                                auto locationSpan = input::LocationSpan(wordPartStartLocation,
                                                                        baseTextIter.getLocation());
                                wordParts.push_back(
                                    arena.allocate<SimpleEscapeSequenceWordPartType>(locationSpan,
                                                                                     '\r'));
                                wordPartStartLocation = baseTextIter.getLocation();
                                break;
                            }
                            case 't':
                            {
                                ++baseTextIter;
                                auto locationSpan = input::LocationSpan(wordPartStartLocation,
                                                                        baseTextIter.getLocation());
                                wordParts.push_back(
                                    arena.allocate<SimpleEscapeSequenceWordPartType>(locationSpan,
                                                                                     '\t'));
                                wordPartStartLocation = baseTextIter.getLocation();
                                break;
                            }
                            case 'v':
                            {
                                ++baseTextIter;
                                auto locationSpan = input::LocationSpan(wordPartStartLocation,
                                                                        baseTextIter.getLocation());
                                wordParts.push_back(
                                    arena.allocate<SimpleEscapeSequenceWordPartType>(locationSpan,
                                                                                     '\v'));
                                wordPartStartLocation = baseTextIter.getLocation();
                                break;
                            }
                            case '\\':
                            case '\'':
                            case '\"':
                            case '?':
                            {
                                char ch = *baseTextIter;
                                ++baseTextIter;
                                auto locationSpan = input::LocationSpan(wordPartStartLocation,
                                                                        baseTextIter.getLocation());
                                wordParts.push_back(
                                    arena.allocate<SimpleEscapeSequenceWordPartType>(locationSpan,
                                                                                     ch));
                                wordPartStartLocation = baseTextIter.getLocation();
                                break;
                            }
                            case 'x': // hex
                            {
                                ++baseTextIter;
                                auto iter2 = baseTextIter;
                                auto value = parseSimpleNumber(iter2, 0x10, 1, 2);
                                if(!value)
                                {
                                    auto locationSpan = input::LocationSpan(
                                        wordPartStartLocation, baseTextIter.getLocation());
                                    wordParts.push_back(
                                        arena.allocate<TextWordPartType>(locationSpan));
                                }
                                else
                                {
                                    baseTextIter = iter2;
                                    auto locationSpan = input::LocationSpan(
                                        wordPartStartLocation, baseTextIter.getLocation());
                                    wordParts.push_back(
                                        arena.allocate<HexEscapeSequenceWordPartType>(locationSpan,
                                                                                      value.get()));
                                }
                                wordPartStartLocation = baseTextIter.getLocation();
                                break;
                            }
                            case '0':
                            case '1':
                            case '2':
                            case '3':
                            case '4':
                            case '5':
                            case '6':
                            case '7': // octal
                            {
                                auto value = parseSimpleNumber(baseTextIter, 8, 1, 3);
                                assert(value); // we already have the first digit
                                auto locationSpan = input::LocationSpan(wordPartStartLocation,
                                                                        baseTextIter.getLocation());
                                wordParts.push_back(arena.allocate<OctalEscapeSequenceWordPartType>(
                                    locationSpan, value.get() & 0xFF));
                                wordPartStartLocation = baseTextIter.getLocation();
                                break;
                            }
                            case 'u':
                            case 'U':
                            {
                                auto escapeChar = *baseTextIter;
                                ++baseTextIter;
                                auto iter2 = baseTextIter;
                                auto value =
                                    parseSimpleNumber(iter2, 0x10, 1, escapeChar == 'U' ? 8 : 4);
                                if(!value)
                                {
                                    auto locationSpan = input::LocationSpan(
                                        wordPartStartLocation, baseTextIter.getLocation());
                                    wordParts.push_back(
                                        arena.allocate<TextWordPartType>(locationSpan));
                                }
                                else
                                {
                                    baseTextIter = iter2;
                                    auto locationSpan = input::LocationSpan(
                                        wordPartStartLocation, baseTextIter.getLocation());
                                    wordParts.push_back(
                                        arena.allocate<UnicodeEscapeSequenceWordPartType>(
                                            locationSpan, util::encodeUTF8(value.get())));
                                }
                                wordPartStartLocation = baseTextIter.getLocation();
                                break;
                            }
                            case '\x01':
                            {
                                ++baseTextIter;
                                auto locationSpan = input::LocationSpan(wordPartStartLocation,
                                                                        baseTextIter.getLocation());
                                if(dialect.duplicateDollarSingleQuoteStringBashParsingFlaws)
                                {
                                    wordParts.push_back(
                                        arena.allocate<BashBugEscapeSequenceWordPartType>(
                                            locationSpan, "\\\x01\x01"));
                                }
                                else
                                {
                                    wordParts.push_back(
                                        arena.allocate<TextWordPartType>(locationSpan));
                                }
                                wordPartStartLocation = baseTextIter.getLocation();
                                break;
                            }
                            case 'c':
                            {
                                ++baseTextIter;
                                switch(*baseTextIter)
                                {
                                case input::eof:
                                case '\'':
                                {
                                    auto locationSpan = input::LocationSpan(
                                        wordPartStartLocation, baseTextIter.getLocation());
                                    wordParts.push_back(
                                        arena.allocate<TextWordPartType>(locationSpan));
                                    break;
                                }
                                case '\\':
                                {
                                    if(dialect.duplicateDollarSingleQuoteStringBashParsingFlaws)
                                    {
                                        ++baseTextIter;
                                        if(*baseTextIter == '\\')
                                            ++baseTextIter;
                                        auto locationSpan = input::LocationSpan(
                                            wordPartStartLocation, baseTextIter.getLocation());
                                        wordParts.push_back(
                                            arena.allocate<SimpleEscapeSequenceWordPartType>(
                                                locationSpan, 0x1C));
                                    }
                                    else
                                    {
                                        auto locationSpan = input::LocationSpan(
                                            wordPartStartLocation, baseTextIter.getLocation());
                                        wordParts.push_back(
                                            arena.allocate<TextWordPartType>(locationSpan));
                                    }
                                    break;
                                }
                                case '\x01':
                                {
                                    ++baseTextIter;
                                    auto locationSpan = input::LocationSpan(
                                        wordPartStartLocation, baseTextIter.getLocation());
                                    if(dialect.duplicateDollarSingleQuoteStringBashParsingFlaws)
                                    {
                                        wordParts.push_back(
                                            arena.allocate<BashBugEscapeSequenceWordPartType>(
                                                locationSpan, "\x01\x01"));
                                    }
                                    else
                                    {
                                        wordParts.push_back(
                                            arena.allocate<SimpleEscapeSequenceWordPartType>(
                                                locationSpan, '\x01'));
                                    }
                                    break;
                                }
                                default:
                                {
                                    int ch = *baseTextIter;
                                    ++baseTextIter;
                                    auto locationSpan = input::LocationSpan(
                                        wordPartStartLocation, baseTextIter.getLocation());
                                    wordParts.push_back(
                                        arena.allocate<SimpleEscapeSequenceWordPartType>(
                                            locationSpan, ch & 0x1F));
                                }
                                }
                                wordPartStartLocation = baseTextIter.getLocation();
                                break;
                            }
                            default:
                            {
                                char ch = *baseTextIter;
                                ++baseTextIter;
                                auto locationSpan = input::LocationSpan(wordPartStartLocation,
                                                                        baseTextIter.getLocation());
                                wordParts.push_back(
                                    arena.allocate<SimpleEscapeSequenceWordPartType>(locationSpan,
                                                                                     ch));
                                wordPartStartLocation = baseTextIter.getLocation();
                                break;
                            }
                            }
                        }
                        else
                        {
                            ++baseTextIter;
                        }
                    }
                    if(wordPartStartLocation != baseTextIter.getLocation())
                        wordParts.push_back(
                            arena.allocate<ast::TextWordPart<ast::WordPart::QuoteKind::
                                                                 EscapeInterpretingSingleQuote>>(
                                input::LocationSpan(wordPartStartLocation,
                                                    baseTextIter.getLocation())));
                    auto closingQuoteStartLocation = baseTextIter.getLocation();
                    textIter = input::LineContinuationRemovingIterator(baseTextIter);
                    ++textIter;
                    wordParts.push_back(
                        arena.allocate<ast::QuoteWordPart<false,
                                                          ast::WordPart::QuoteKind::
                                                              EscapeInterpretingSingleQuote>>(
                            input::LocationSpan(closingQuoteStartLocation,
                                                textIter.getLocation())));
                }
                else
                {
                    UNIMPLEMENTED();
                }
            }
            else
            {
                UNIMPLEMENTED();
            }
        }
        if(wordParts.empty())
            return parserErrorStaticString("missing word", textIter);
        if(checkForReservedWords && wordParts.size() == 1)
        {
            auto &wordPart = wordParts.front();
            if(util::dynamic_pointer_cast<ast::TextWordPart<ast::WordPart::QuoteKind::Unquoted>>(
                   wordPart))
            {
                auto result = stringToReservedWord(wordPart->getSourceText());
                if(result.is<ReservedWord>())
                {
                    wordPart = ast::GenericReservedWordPart::make(
                        arena, wordPart->location, result.get<ReservedWord>());
                }
            }
        }
        return parserSuccess(arena.allocate<ast::Word>(
            input::LocationSpan(wordStartLocation, textIter.getLocation()), std::move(wordParts)));
    }
#if 0
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
#endif
#warning finish
public:
    void test();
};
}
}

#endif /* PARSER_PARSER_H_ */
