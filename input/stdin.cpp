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

#include "stdin.h"
#include "istream.h"
#include <iostream>

namespace
{
inline bool isStdInATerminalImplementation() noexcept;
}

namespace quick_shell
{
namespace input
{
std::unique_ptr<Input> makeStdInInput(const InputStyle &inputStyle, bool retryAfterEOF)
{
    return std::unique_ptr<Input>(new IStreamInput("stdin", inputStyle, std::cin, retryAfterEOF));
}

bool isStdInATerminal() noexcept
{
    return isStdInATerminalImplementation();
}
}
}

#if defined(__unix)
#include <unistd.h>
namespace
{
inline bool isStdInATerminalImplementation() noexcept
{
    return isatty(STDIN_FILENO);
}
}
#elif defined(_WIN32)
#include <io.h>
#include <stdio.h>
namespace
{
inline bool isStdInATerminalImplementation() noexcept
{
    return _isatty(STDIN_FILENO);
}
}
#else
#error unimplemented platform
#endif
