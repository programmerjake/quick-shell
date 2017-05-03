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
#ifndef AST_WORD_H_
#define AST_WORD_H_

#include <vector>
#include "word_or_redirection.h"

namespace quick_shell
{
namespace ast
{
struct WordPart;

struct Word final : public WordOrRedirection
{
    std::vector<util::ArenaPtr<WordPart>> wordParts;
    Word(const input::LocationSpan &location, std::vector<util::ArenaPtr<WordPart>> wordParts)
        : WordOrRedirection(location), wordParts(std::move(wordParts))
    {
    }
    Word(const input::LocationSpan &location,
         std::initializer_list<util::ArenaPtr<WordPart>> wordParts)
        : WordOrRedirection(location), wordParts(wordParts)
    {
    }
    explicit Word(const input::LocationSpan &location) : WordOrRedirection(location), wordParts()
    {
    }
    virtual util::ArenaPtr<WordOrRedirection> duplicate(util::Arena &arena) const override
    {
        return arena.allocate<Word>(*this);
    }
    virtual util::ArenaPtr<WordOrRedirection> duplicateRecursive(util::Arena &arena) const override;
    virtual void dump(std::ostream &os, ASTDumpState &dumpState) const override;
};
}
}

#endif /* AST_WORD_H_ */
