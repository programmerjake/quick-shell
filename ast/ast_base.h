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
#include "../util/arena.h"
#include "../input/location.h"

namespace quick_shell
{
namespace ast
{
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
    std::string getSourceText(std::string bufferSource) const
    {
        return location.getTextInputText(std::move(bufferSource));
    }
    std::string getSourceText() const
    {
        return location.getTextInputText();
    }
};
}
}

#endif /* AST_AST_BASE_H_ */
