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
#ifndef AST_AST_BASE_H_
#define AST_AST_BASE_H_

#include <string>
#include <utility>
#include <iosfwd>
#include "../util/arena.h"
#include "../input/location.h"
#include "../util/string_view.h"

namespace quick_shell
{
namespace ast
{
class ASTDumpState final
{
public:
    struct IndentState final
    {
        std::size_t indentDepth = 0;
        std::string indentString = "    ";
        void push() noexcept
        {
            indentDepth++;
        }
        void pop() noexcept
        {
            indentDepth--;
        }
        friend std::ostream &operator<<(std::ostream &os, const IndentState &indentState);
    };
    class PushIndent
    {
        PushIndent(const PushIndent &) = delete;
        PushIndent &operator=(const PushIndent &) = delete;

    private:
        ASTDumpState &dumpState;

    public:
        explicit PushIndent(ASTDumpState &dumpState) noexcept : dumpState(dumpState)
        {
            dumpState.indent.push();
        }
        ~PushIndent()
        {
            dumpState.indent.pop();
        }
    };
    class EscapedQuotedString final
    {
        friend class ASTDumpState;

    private:
        util::string_view string;

    private:
        constexpr explicit EscapedQuotedString(util::string_view string) noexcept : string(string)
        {
        }

    public:
        friend std::ostream &operator<<(std::ostream &os, const EscapedQuotedString &v);
    };
    constexpr static EscapedQuotedString escapedQuotedString(util::string_view string) noexcept
    {
    	return EscapedQuotedString(string);
    }
    IndentState indent;
};

template <typename T>
struct ASTBase
{
    input::LocationSpan location;
    ASTBase(const ASTBase &) = default;
    ASTBase(ASTBase &&) = delete;
    ASTBase &operator=(const ASTBase &) = delete;
    ASTBase &operator=(ASTBase &&) = delete;
    explicit ASTBase(const input::LocationSpan &location) noexcept : location(location)
    {
    }
    virtual ~ASTBase() = default;
    virtual util::ArenaPtr<T> duplicate(util::Arena &arena) const = 0;
    virtual util::ArenaPtr<T> duplicateRecursive(util::Arena &arena) const
    {
        return duplicate(arena);
    }
    std::string getSourceText(std::string bufferSource) const
    {
        return location.getTextInputText(std::move(bufferSource));
    }
    std::string getSourceText() const
    {
        return location.getTextInputText();
    }
    virtual void dump(std::ostream &os, ASTDumpState &dumpState) const = 0;
};
}
}

#endif /* AST_AST_BASE_H_ */
