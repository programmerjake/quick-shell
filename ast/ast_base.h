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

#include "../util/arena.h"

namespace quick_shell
{
namespace ast
{
template <typename T>
struct ASTBase
{
    ASTBase(const ASTBase &) = default;
    ASTBase(ASTBase &&) = delete;
    ASTBase &operator=(const ASTBase &) = delete;
    ASTBase &operator=(ASTBase &&) = delete;
    ASTBase() = default;
    virtual ~ASTBase() = default;
    virtual util::ArenaPtr<T> duplicate(util::Arena &arena) const = 0;
};
}
}

#endif /* AST_AST_BASE_H_ */
