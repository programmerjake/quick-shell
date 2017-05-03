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
#ifndef PARSER_RESERVED_WORD_H_
#define PARSER_RESERVED_WORD_H_

#include <cassert>
#include <algorithm>
#include <tuple>
#include "../util/string_view.h"
#include "../util/compiler_intrinsics.h"
#include "../util/variant.h"

namespace quick_shell
{
namespace parser
{
enum class ReservedWord
{
    ExMark, // "!"
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
    Then, // "then"
    Time, // "time"
    Until, // "until"
    While, // "while"
    LBrace, // "{"
    RBrace, // "}"
};

inline util::string_view getReservedWordString(ReservedWord v) noexcept
{
    switch(v)
    {
    case ReservedWord::ExMark:
        return "!";
    case ReservedWord::DoubleLBracket:
        return "[[";
    case ReservedWord::DoubleRBracket:
        return "]]";
    case ReservedWord::Case:
        return "case";
    case ReservedWord::Coproc:
        return "coproc";
    case ReservedWord::Do:
        return "do";
    case ReservedWord::Done:
        return "done";
    case ReservedWord::ElIf:
        return "elif";
    case ReservedWord::Else:
        return "else";
    case ReservedWord::Esac:
        return "esac";
    case ReservedWord::Fi:
        return "fi";
    case ReservedWord::For:
        return "for";
    case ReservedWord::Function:
        return "function";
    case ReservedWord::If:
        return "if";
    case ReservedWord::In:
        return "in";
    case ReservedWord::Select:
        return "select";
    case ReservedWord::Time:
        return "time";
    case ReservedWord::Then:
        return "then";
    case ReservedWord::Until:
        return "until";
    case ReservedWord::While:
        return "while";
    case ReservedWord::LBrace:
        return "{";
    case ReservedWord::RBrace:
        return "}";
    }
    UNREACHABLE();
    return "";
}

inline util::string_view getReservedWordName(ReservedWord v) noexcept
{
    switch(v)
    {
    case ReservedWord::ExMark:
        return "ExMark";
    case ReservedWord::DoubleLBracket:
        return "DoubleLBracket";
    case ReservedWord::DoubleRBracket:
        return "DoubleRBracket";
    case ReservedWord::Case:
        return "Case";
    case ReservedWord::Coproc:
        return "Coproc";
    case ReservedWord::Do:
        return "Do";
    case ReservedWord::Done:
        return "Done";
    case ReservedWord::ElIf:
        return "ElIf";
    case ReservedWord::Else:
        return "Else";
    case ReservedWord::Esac:
        return "Esac";
    case ReservedWord::Fi:
        return "Fi";
    case ReservedWord::For:
        return "For";
    case ReservedWord::Function:
        return "Function";
    case ReservedWord::If:
        return "If";
    case ReservedWord::In:
        return "In";
    case ReservedWord::Select:
        return "Select";
    case ReservedWord::Time:
        return "Time";
    case ReservedWord::Then:
        return "Then";
    case ReservedWord::Until:
        return "Until";
    case ReservedWord::While:
        return "While";
    case ReservedWord::LBrace:
        return "LBrace";
    case ReservedWord::RBrace:
        return "RBrace";
    }
    UNREACHABLE();
    return "";
}

inline util::variant<ReservedWord> stringToReservedWord(util::string_view v) noexcept
{
    struct StringAndReservedWord
    {
        util::string_view string;
        ReservedWord reservedWord;
        constexpr StringAndReservedWord(util::string_view string,
                                        ReservedWord reservedWord) noexcept
            : string(string),
              reservedWord(reservedWord)
        {
        }
    };
    struct Comparer
    {
        bool operator()(StringAndReservedWord a, StringAndReservedWord b) const noexcept
        {
            return a.string < b.string;
        }
        bool operator()(util::string_view a, StringAndReservedWord b) const noexcept
        {
            return a < b.string;
        }
        bool operator()(StringAndReservedWord a, util::string_view b) const noexcept
        {
            return a.string < b;
        }
        bool operator()(util::string_view a, util::string_view b) const noexcept
        {
            return a < b;
        }
    };
    static const StringAndReservedWord mappingTable[] = {
        {"!", ReservedWord::ExMark},
        {"[[", ReservedWord::DoubleLBracket},
        {"]]", ReservedWord::DoubleRBracket},
        {"case", ReservedWord::Case},
        {"coproc", ReservedWord::Coproc},
        {"do", ReservedWord::Do},
        {"done", ReservedWord::Done},
        {"elif", ReservedWord::ElIf},
        {"else", ReservedWord::Else},
        {"esac", ReservedWord::Esac},
        {"fi", ReservedWord::Fi},
        {"for", ReservedWord::For},
        {"function", ReservedWord::Function},
        {"if", ReservedWord::If},
        {"in", ReservedWord::In},
        {"select", ReservedWord::Select},
        {"then", ReservedWord::Then},
        {"time", ReservedWord::Time},
        {"until", ReservedWord::Until},
        {"while", ReservedWord::While},
        {"{", ReservedWord::LBrace},
        {"}", ReservedWord::RBrace},
    };
    static const bool isSorted =
        std::is_sorted(std::begin(mappingTable), std::end(mappingTable), Comparer());
    assert(isSorted);
    auto results =
        std::equal_range(std::begin(mappingTable), std::end(mappingTable), v, Comparer());
    auto *first = std::get<0>(results);
    auto *last = std::get<1>(results);
    if(first == last)
        return util::variant<ReservedWord>();
    return util::variant<ReservedWord>(first->reservedWord);
}
}
}

#endif /* PARSER_RESERVED_WORD_H_ */
