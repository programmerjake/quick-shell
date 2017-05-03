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
#include "word.h"
#include "word_part.h"
#include <ostream>

namespace quick_shell
{
namespace ast
{
util::ArenaPtr<WordOrRedirection> Word::duplicateRecursive(util::Arena &arena) const
{
    auto retval = arena.allocate<Word>(*this);
    for(auto &wordPart : retval->wordParts)
        wordPart = wordPart->duplicateRecursive(arena);
    return retval;
}

void Word::dump(std::ostream &os, ASTDumpState &dumpState) const
{
    os << dumpState.indent;
    ASTDumpState::PushIndent pushIndent(dumpState);
    os << location << ": Word" << std::endl;
    for(auto &wordPart : wordParts)
    {
    	wordPart->dump(os, dumpState);
    }
}
}
}
