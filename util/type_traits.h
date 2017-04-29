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
#ifndef UTIL_TYPE_TRAITS_H_
#define UTIL_TYPE_TRAITS_H_

#include <type_traits>

namespace quick_shell
{
namespace util
{
template <typename T>
struct IsFunctionPointer
{
    static constexpr bool value = false;
};

template <typename R, typename... Args>
struct IsFunctionPointer<R (*)(Args...)>
{
    static constexpr bool value = true;
};
}
}

#endif /* UTIL_TYPE_TRAITS_H_ */
