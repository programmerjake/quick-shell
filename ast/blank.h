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
#ifndef AST_BLANK_H_
#define AST_BLANK_H_

#include <cassert>
#include "ast_base.h"

namespace quick_shell
{
namespace ast
{
struct BlankOrEmpty : public ASTBase<BlankOrEmpty>
{
    using ASTBase<BlankOrEmpty>::ASTBase;
    bool isEmpty() noexcept
    {
        return location.size() == 0;
    }
    virtual util::ArenaPtr<BlankOrEmpty> duplicate(util::Arena &arena) const override
    {
        return arena.allocate<BlankOrEmpty>(*this);
    }
    virtual void dump(std::ostream &os, ASTDumpState &dumpState) const override;
};

struct Blank final : public BlankOrEmpty
{
    explicit Blank(const input::LocationSpan &location) noexcept : BlankOrEmpty(location)
    {
    	assert(!isEmpty());
    }
    virtual util::ArenaPtr<BlankOrEmpty> duplicate(util::Arena &arena) const override
    {
        return arena.allocate<Blank>(*this);
    }
    virtual void dump(std::ostream &os, ASTDumpState &dumpState) const override;
};
}
}

#endif /* AST_BLANK_H_ */
