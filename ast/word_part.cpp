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
#include "word_part.h"

namespace quick_shell
{
namespace ast
{
void AssignmentVariableNameWordPart::dump(std::ostream &os, ASTDumpState &dumpState) const
{
    os << dumpState.indent << location << ": AssignmentVariableNameWordPart: "
       << ASTDumpState::escapedQuotedString(location.getTextInputText()) << std::endl;
}

void AssignmentEqualSignWordPart::dump(std::ostream &os, ASTDumpState &dumpState) const
{
    os << dumpState.indent << location << ": AssignmentEqualSignWordPart: "
       << ASTDumpState::escapedQuotedString(location.getTextInputText()) << std::endl;
}
}
}