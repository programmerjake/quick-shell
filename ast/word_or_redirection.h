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
#ifndef AST_WORD_OR_REDIRECTION_H_
#define AST_WORD_OR_REDIRECTION_H_

#include "ast_base.h"

namespace quick_shell
{
namespace ast
{
struct WordOrRedirection : public ASTBase<WordOrRedirection>
{
    using ASTBase<WordOrRedirection>::ASTBase;
};
}
}

#endif /* AST_WORD_OR_REDIRECTION_H_ */
