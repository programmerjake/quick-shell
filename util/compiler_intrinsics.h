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
#ifndef UTIL_COMPILER_INTRINSICS_H_
#define UTIL_COMPILER_INTRINSICS_H_

#include <cassert>
#if defined(__clang__) || (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)
#define BUILTIN_UNREACHABLE() __builtin_unreachable()
#else
#error BUILTIN_UNREACHABLE not implemented for this platform
#endif

#if defined(__clang__) || (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)
#define BUILTIN_TRAP() __builtin_trap()
#else
#error BUILTIN_TRAP not implemented for this platform
#endif

#if(defined(__clang__) && (__clang_major__ > 3 || (__clang_major__ == 3 && __clang_minor__ >= 3))) \
    || (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)
#define PRAGMA_WARNING_STR_HELPER(x) _Pragma(#x)
#define PRAGMA_WARNING_STR(x) PRAGMA_WARNING_STR_HELPER(GCC warning x)
#define PRAGMA_WARNING(x) PRAGMA_WARNING_STR(#x)
#else
#define PRAGMA_WARNING_STR(x)
#define PRAGMA_WARNING(x)
#endif

#ifdef NDEBUG
#define UNREACHABLE() BUILTIN_UNREACHABLE()
#define UNIMPLEMENTED(x) PRAGMA_WARNING_STR("unimplemented: " #x) BUILTIN_TRAP()
#else
#define UNREACHABLE() assert(!"unreachable")
#define UNIMPLEMENTED_HELPER(x) PRAGMA_WARNING_STR(#x) assert(!#x)
#define UNIMPLEMENTED(x) UNIMPLEMENTED_HELPER(unimplemented : x)
#endif

#endif /* UTIL_COMPILER_INTRINSICS_H_ */
