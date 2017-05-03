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
#ifndef AST_WORD_PART_H_
#define AST_WORD_PART_H_

#include <string>
#include <cassert>
#include <utility>
#include <ostream>
#include "ast_base.h"
#include "../util/string_view.h"
#include "../util/compiler_intrinsics.h"
#include "../parser/reserved_word.h"

namespace quick_shell
{
namespace ast
{
using parser::ReservedWord;

struct WordPart : public ASTBase<WordPart>
{
    using ASTBase<WordPart>::ASTBase;
    enum class QuoteKind
    {
        Unquoted,
        SingleQuote,
        DoubleQuote,
        EscapeInterpretingSingleQuote,
        LocalizedDoubleQuote,
    };
    static util::string_view getQuoteKindString(QuoteKind kind) noexcept
    {
        switch(kind)
        {
        case QuoteKind::Unquoted:
            return "Unquoted";
        case QuoteKind::SingleQuote:
            return "SingleQuote";
        case QuoteKind::DoubleQuote:
            return "DoubleQuote";
        case QuoteKind::EscapeInterpretingSingleQuote:
            return "EscapeInterpretingSingleQuote";
        case QuoteKind::LocalizedDoubleQuote:
            return "LocalizedDoubleQuote";
        }
        UNREACHABLE();
        return "";
    }
    static util::string_view getQuotePrefix(QuoteKind kind) noexcept
    {
        switch(kind)
        {
        case QuoteKind::Unquoted:
            return "";
        case QuoteKind::SingleQuote:
            return "\'";
        case QuoteKind::DoubleQuote:
            return "\"";
        case QuoteKind::EscapeInterpretingSingleQuote:
            return "$\'";
        case QuoteKind::LocalizedDoubleQuote:
            return "$\"";
        }
        UNREACHABLE();
        return "";
    }
    util::string_view getQuotePrefix() const noexcept
    {
        return getQuotePrefix(getQuoteKind());
    }
    static util::string_view getQuoteSuffix(QuoteKind kind) noexcept
    {
        switch(kind)
        {
        case QuoteKind::Unquoted:
            return "";
        case QuoteKind::SingleQuote:
            return "\'";
        case QuoteKind::DoubleQuote:
            return "\"";
        case QuoteKind::EscapeInterpretingSingleQuote:
            return "\'";
        case QuoteKind::LocalizedDoubleQuote:
            return "\"";
        }
        UNREACHABLE();
        return "";
    }
    util::string_view getQuoteSuffix() const noexcept
    {
        return getQuoteSuffix(getQuoteKind());
    }
    virtual QuoteKind getQuoteKind() const noexcept = 0;
    enum class QuotePart
    {
        Start,
        Stop,
        Other,
    };
    virtual QuotePart getQuotePart() const noexcept
    {
        return QuotePart::Other;
    }
};

struct GenericQuoteWordPart : public WordPart
{
    using WordPart::WordPart;
};

template <bool isStart, WordPart::QuoteKind quoteKind>
struct QuoteWordPart final : public GenericQuoteWordPart
{
    static_assert(quoteKind != QuoteKind::Unquoted, "");
    using GenericQuoteWordPart::GenericQuoteWordPart;
    virtual QuotePart getQuotePart() const noexcept override
    {
        return isStart ? QuotePart::Start : QuotePart::Stop;
    }
    virtual QuoteKind getQuoteKind() const noexcept override
    {
        return quoteKind;
    }
    virtual util::ArenaPtr<WordPart> duplicate(util::Arena &arena) const override
    {
        return arena.allocate<QuoteWordPart>(*this);
    }
    virtual void dump(std::ostream &os, ASTDumpState &dumpState) const override
    {
        os << dumpState.indent << location << ": QuoteWordPart<" << (isStart ? "Start" : "Stop")
           << ", " << getQuoteKindString(quoteKind)
           << ">: " << ASTDumpState::escapedQuotedString(getRawSourceText()) << std::endl;
    }
};

struct GenericTextWordPart : public WordPart
{
    using WordPart::WordPart;
    virtual QuotePart getQuotePart() const noexcept override final
    {
        return QuotePart::Other;
    }
};

struct GenericVariableNameWordPart : public GenericTextWordPart
{
    using GenericTextWordPart::GenericTextWordPart;
};

struct AssignmentVariableNameWordPart final : public GenericVariableNameWordPart
{
    using GenericVariableNameWordPart::GenericVariableNameWordPart;
    virtual QuoteKind getQuoteKind() const noexcept override
    {
        return QuoteKind::Unquoted;
    }
    virtual util::ArenaPtr<WordPart> duplicate(util::Arena &arena) const override
    {
        return arena.allocate<AssignmentVariableNameWordPart>(*this);
    }
    virtual void dump(std::ostream &os, ASTDumpState &dumpState) const override;
};

struct AssignmentEqualSignWordPart final : public GenericTextWordPart
{
    using GenericTextWordPart::GenericTextWordPart;
    virtual QuoteKind getQuoteKind() const noexcept override
    {
        return QuoteKind::Unquoted;
    }
    virtual util::ArenaPtr<WordPart> duplicate(util::Arena &arena) const override
    {
        return arena.allocate<AssignmentEqualSignWordPart>(*this);
    }
    virtual void dump(std::ostream &os, ASTDumpState &dumpState) const override;
};

template <WordPart::QuoteKind quoteKind>
struct TextWordPart final : public GenericTextWordPart
{
    using GenericTextWordPart::GenericTextWordPart;
    virtual QuoteKind getQuoteKind() const noexcept override
    {
        return quoteKind;
    }
    virtual util::ArenaPtr<WordPart> duplicate(util::Arena &arena) const override
    {
        return arena.allocate<TextWordPart>(*this);
    }
    virtual void dump(std::ostream &os, ASTDumpState &dumpState) const override
    {
        os << dumpState.indent << location << ": TextWordPart<" << getQuoteKindString(quoteKind)
           << ">: " << ASTDumpState::escapedQuotedString(getRawSourceText()) << std::endl;
    }
};

struct GenericReservedWordPart : public GenericTextWordPart
{
    using GenericTextWordPart::GenericTextWordPart;
    virtual ReservedWord getReservedWord() const noexcept = 0;
    virtual QuoteKind getQuoteKind() const noexcept override final
    {
        return QuoteKind::Unquoted;
    }
    static util::ArenaPtr<GenericReservedWordPart> make(util::Arena &arena,
                                                        const input::LocationSpan &location,
                                                        ReservedWord reservedWord);
};

template <ReservedWord reservedWord>
struct ReservedWordPart final : public GenericReservedWordPart
{
    using GenericReservedWordPart::GenericReservedWordPart;
    virtual ReservedWord getReservedWord() const noexcept override
    {
        return reservedWord;
    }
    virtual util::ArenaPtr<WordPart> duplicate(util::Arena &arena) const override
    {
        return arena.allocate<ReservedWordPart>(*this);
    }
    virtual void dump(std::ostream &os, ASTDumpState &dumpState) const override
    {
        os << dumpState.indent << location << ": ReservedWordPart<"
           << getReservedWordName(reservedWord)
           << ">: " << ASTDumpState::escapedQuotedString(getRawSourceText()) << std::endl;
    }
};

inline util::ArenaPtr<GenericReservedWordPart> GenericReservedWordPart::make(
    util::Arena &arena, const input::LocationSpan &location, ReservedWord reservedWord)
{
    switch(reservedWord)
    {
    case ReservedWord::ExMark:
        return arena.allocate<ReservedWordPart<ReservedWord::ExMark>>(location);
    case ReservedWord::DoubleLBracket:
        return arena.allocate<ReservedWordPart<ReservedWord::DoubleLBracket>>(location);
    case ReservedWord::DoubleRBracket:
        return arena.allocate<ReservedWordPart<ReservedWord::DoubleRBracket>>(location);
    case ReservedWord::Case:
        return arena.allocate<ReservedWordPart<ReservedWord::Case>>(location);
    case ReservedWord::Coproc:
        return arena.allocate<ReservedWordPart<ReservedWord::Coproc>>(location);
    case ReservedWord::Do:
        return arena.allocate<ReservedWordPart<ReservedWord::Do>>(location);
    case ReservedWord::Done:
        return arena.allocate<ReservedWordPart<ReservedWord::Done>>(location);
    case ReservedWord::ElIf:
        return arena.allocate<ReservedWordPart<ReservedWord::ElIf>>(location);
    case ReservedWord::Else:
        return arena.allocate<ReservedWordPart<ReservedWord::Else>>(location);
    case ReservedWord::Esac:
        return arena.allocate<ReservedWordPart<ReservedWord::Esac>>(location);
    case ReservedWord::Fi:
        return arena.allocate<ReservedWordPart<ReservedWord::Fi>>(location);
    case ReservedWord::For:
        return arena.allocate<ReservedWordPart<ReservedWord::For>>(location);
    case ReservedWord::Function:
        return arena.allocate<ReservedWordPart<ReservedWord::Function>>(location);
    case ReservedWord::If:
        return arena.allocate<ReservedWordPart<ReservedWord::If>>(location);
    case ReservedWord::In:
        return arena.allocate<ReservedWordPart<ReservedWord::In>>(location);
    case ReservedWord::Select:
        return arena.allocate<ReservedWordPart<ReservedWord::Select>>(location);
    case ReservedWord::Then:
        return arena.allocate<ReservedWordPart<ReservedWord::Then>>(location);
    case ReservedWord::Time:
        return arena.allocate<ReservedWordPart<ReservedWord::Time>>(location);
    case ReservedWord::Until:
        return arena.allocate<ReservedWordPart<ReservedWord::Until>>(location);
    case ReservedWord::While:
        return arena.allocate<ReservedWordPart<ReservedWord::While>>(location);
    case ReservedWord::LBrace:
        return arena.allocate<ReservedWordPart<ReservedWord::LBrace>>(location);
    case ReservedWord::RBrace:
        return arena.allocate<ReservedWordPart<ReservedWord::RBrace>>(location);
    }
    UNREACHABLE();
    return nullptr;
}

struct GenericEscapeSequenceWordPart : public WordPart
{
    using WordPart::WordPart;
    virtual util::string_view getValue() const noexcept = 0;
    virtual QuotePart getQuotePart() const noexcept override final
    {
        return QuotePart::Other;
    }
};

struct GenericSimpleEscapeSequenceWordPart : public GenericEscapeSequenceWordPart
{
    char value;
    explicit GenericSimpleEscapeSequenceWordPart(const input::LocationSpan &location,
                                                 char value) noexcept
        : GenericEscapeSequenceWordPart(location),
          value(value)
    {
    }
    virtual util::string_view getValue() const noexcept override final
    {
        return util::string_view(&value, 1);
    }
};

template <WordPart::QuoteKind quoteKind>
struct SimpleEscapeSequenceWordPart final : public GenericSimpleEscapeSequenceWordPart
{
    using GenericSimpleEscapeSequenceWordPart::GenericSimpleEscapeSequenceWordPart;
    virtual QuoteKind getQuoteKind() const noexcept override
    {
        return quoteKind;
    }
    virtual util::ArenaPtr<WordPart> duplicate(util::Arena &arena) const override
    {
        return arena.allocate<SimpleEscapeSequenceWordPart>(*this);
    }
    virtual void dump(std::ostream &os, ASTDumpState &dumpState) const override
    {
        os << dumpState.indent << location << ": SimpleEscapeSequenceWordPart<"
           << getQuoteKindString(quoteKind)
           << ">(value=" << ASTDumpState::escapedQuotedString(getValue())
           << "): " << ASTDumpState::escapedQuotedString(getRawSourceText()) << std::endl;
    }
};
}
}

#endif /* AST_WORD_PART_H_ */
