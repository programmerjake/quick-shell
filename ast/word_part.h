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
#include "ast_base.h"
#include "../util/string_view.h"
#include "../util/compiler_intrinsics.h"

namespace quick_shell
{
namespace ast
{
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
};

struct GenericTextWordPart : public WordPart
{
    using WordPart::WordPart;
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
};
}
}

#endif /* AST_WORD_PART_H_ */
