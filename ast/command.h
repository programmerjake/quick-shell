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
#ifndef AST_COMMAND_H_
#define AST_COMMAND_H_

#include <vector>
#include <utility>
#include "ast_base.h"
#include "word_or_redirection.h"
#include "blank.h"
#include "comment.h"

namespace quick_shell
{
namespace ast
{
struct Command : public ASTBase<Command>
{
    using ASTBase<Command>::ASTBase;
};

struct SimpleCommand final : public Command
{
    struct Part final
    {
        util::ArenaPtr<WordOrRedirection> wordOrRedirection;
        util::ArenaPtr<BlankOrEmpty> followingBlanks;
        constexpr Part(util::ArenaPtr<WordOrRedirection> wordOrRedirection,
                       util::ArenaPtr<BlankOrEmpty> followingBlanks) noexcept
            : wordOrRedirection(std::move(wordOrRedirection)),
              followingBlanks(std::move(followingBlanks))
        {
        }
        constexpr Part() noexcept : wordOrRedirection(), followingBlanks()
        {
        }
    };
    util::ArenaPtr<BlankOrEmpty> initialBlanks;
    std::vector<Part> parts;
    util::ArenaPtr<Comment> finalComment;
    SimpleCommand(const input::LocationSpan &location,
                  util::ArenaPtr<BlankOrEmpty> initialBlanks,
                  std::vector<Part> parts,
                  util::ArenaPtr<Comment> finalComment) noexcept
        : Command(location),
          initialBlanks(std::move(initialBlanks)),
          parts(std::move(parts)),
          finalComment(std::move(finalComment))
    {
    }
    virtual util::ArenaPtr<Command> duplicate(util::Arena &arena) const override
    {
        return arena.allocate<Command>(*this);
    }
    virtual void dump(std::ostream &os, ASTDumpState &dumpState) const override;
};
}
}

#endif /* AST_COMMAND_H_ */
