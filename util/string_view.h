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
#ifndef UTIL_STRING_VIEW_H_
#define UTIL_STRING_VIEW_H_

#include <string>
#include <iterator>
#include <initializer_list>
#include <utility>
#include <stdexcept>
#include <ostream>

namespace quick_shell
{
namespace util
{
template <typename CharType, typename TraitsType = std::char_traits<CharType>>
class basic_string_view
{
public:
    typedef TraitsType traits_type;
    typedef CharType value_type;
    typedef CharType *pointer;
    typedef const CharType *const_pointer;
    typedef CharType &reference;
    typedef const CharType &const_reference;
    typedef const CharType *const_iterator;
    typedef const_iterator iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef const_reverse_iterator reverse_iterator;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    static constexpr std::size_t npos = static_cast<std::size_t>(-1);

private:
    const CharType *stringPointer;
    std::size_t stringSize;

private:
    static constexpr std::size_t constexprMin(std::size_t a, std::size_t b) noexcept
    {
        return a < b ? a : b;
    }

public:
    constexpr basic_string_view() noexcept : stringPointer(nullptr), stringSize(0)
    {
    }
    constexpr basic_string_view(const basic_string_view &) noexcept = default;
    template <typename Allocator>
    constexpr basic_string_view(
        const std::basic_string<CharType, TraitsType, Allocator> &str) noexcept
        : stringPointer(str.data()), stringSize(str.size())
    {
    }
    constexpr basic_string_view(const CharType *str, std::size_t count) noexcept
        : stringPointer(str), stringSize(count)
    {
    }
    basic_string_view(const CharType *str)
        : stringPointer(str), stringSize(traits_type::length(str))
    {
    }
    basic_string_view &operator=(const basic_string_view &) noexcept = default;
    constexpr const_iterator begin() const noexcept
    {
        return stringPointer;
    }
    constexpr const_iterator cbegin() const noexcept
    {
        return stringPointer;
    }
    constexpr const_iterator end() const noexcept
    {
        return stringPointer + stringSize;
    }
    constexpr const_iterator cend() const noexcept
    {
        return stringPointer + stringSize;
    }
    constexpr const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }
    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }
    constexpr const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator(begin());
    }
    constexpr const_reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator(begin());
    }
    constexpr const CharType &at(std::size_t index) const
    {
        return index >= stringSize ?
                   throw std::out_of_range("out of range in util::basic_string_view::at") :
                   stringPointer[index];
    }
    constexpr const CharType &operator[](std::size_t index) const noexcept
    {
        return stringPointer[index];
    }
    constexpr const CharType &front() const noexcept
    {
        return stringPointer[0];
    }
    constexpr const CharType &back() const noexcept
    {
        return stringPointer[stringSize - 1];
    }
    constexpr const CharType *data() const noexcept
    {
        return stringPointer;
    }
    constexpr std::size_t size() const noexcept
    {
        return stringSize;
    }
    constexpr std::size_t length() const noexcept
    {
        return stringSize;
    }
    constexpr std::size_t max_size() const noexcept
    {
        return static_cast<std::size_t>(-1) / sizeof(CharType);
    }
    constexpr bool empty() const noexcept
    {
        return stringSize == 0;
    }
    void remove_prefix(std::size_t n) noexcept
    {
        stringPointer += n;
        stringSize -= n;
    }
    void remove_suffix(std::size_t n) noexcept
    {
        stringSize -= n;
    }
    void swap(basic_string_view &rt) noexcept
    {
        basic_string_view temp = *this;
        *this = rt;
        rt = temp;
    }
    std::size_t copy(CharType *dest, std::size_t count, std::size_t pos = 0) const
    {
        if(pos > count)
            throw std::out_of_range("out of range in util::basic_string_view::copy");
        count = constexprMin(count, stringSize - pos);
        for(std::size_t i = 0; i < count; i++)
            dest[i] = stringPointer[i + pos];
        return count;
    }
    constexpr basic_string_view substr(std::size_t pos = 0, std::size_t count = npos) const
    {
        return pos > stringSize ?
                   throw std::out_of_range("out of range in util::basic_string_view::substr") :
                   basic_string_view(stringPointer + pos, constexprMin(count, stringSize - pos));
    }

private:
    constexpr int compareHelper(int compareResult, basic_string_view rt) const noexcept
    {
        return compareResult != 0 ?
                   compareResult :
                   stringSize > rt.stringSize ? 1 : stringSize < rt.stringSize ? -1 : 0;
    }

public:
    constexpr int compare(basic_string_view rt) const noexcept
    {
        return compareHelper(
            traits_type::compare(
                stringPointer, rt.stringPointer, constexprMin(stringSize, rt.stringSize)),
            rt);
    }
    constexpr int compare(std::size_t pos1, std::size_t count1, basic_string_view rt) const
    {
        return substr(pos1, count1).compare(rt);
    }
    constexpr int compare(std::size_t pos1,
                          std::size_t count1,
                          basic_string_view rt,
                          std::size_t pos2,
                          std::size_t count2) const
    {
        return substr(pos1, count1).compare(rt.substr(pos2, count2));
    }
    constexpr int compare(const CharType *rt) const
    {
        return compare(basic_string_view(rt));
    }
    constexpr int compare(std::size_t pos1, std::size_t count1, const CharType *rt) const
    {
        return substr(pos1, count1).compare(rt);
    }
    constexpr int compare(std::size_t pos1,
                          std::size_t count1,
                          basic_string_view rt,
                          std::size_t count2) const
    {
        return substr(pos1, count1).compare(basic_string_view(rt, count2));
    }
    std::size_t find(basic_string_view v, std::size_t pos = 0) const noexcept
    {
        if(pos > stringSize)
            return npos;
        for(; stringSize - pos < v.stringSize; pos++)
        {
            bool found = true;
            for(std::size_t i = 0; i < v.stringSize; i++)
            {
                if(!traits_type::eq(stringPointer[i + pos], v.stringPointer[i]))
                {
                    found = false;
                    break;
                }
            }
            if(found)
                return pos;
        }
        return npos;
    }
    std::size_t find(CharType c, std::size_t pos = 0) const noexcept
    {
        return find(basic_string_view(std::addressof(c), 1), pos);
    }
    std::size_t find(const CharType *s, std::size_t pos, std::size_t count) const noexcept
    {
        return find(basic_string_view(s, count), pos);
    }
    std::size_t find(const CharType *s, std::size_t pos = 0) const
    {
        return find(basic_string_view(s), pos);
    }
    std::size_t rfind(basic_string_view v, std::size_t pos = npos) const noexcept
    {
        if(v.stringSize > stringSize)
            return npos;
        pos = constexprMin(pos, stringSize - v.stringSize);
        for(std::size_t i = 0, count = pos; i < count; i++, pos--)
        {
            bool found = true;
            for(std::size_t i = 0; i < v.stringSize; i++)
            {
                if(!traits_type::eq(stringPointer[i + pos], v.stringPointer[i]))
                {
                    found = false;
                    break;
                }
            }
            if(found)
                return pos;
        }
        return npos;
    }
    std::size_t rfind(CharType c, std::size_t pos = npos) const noexcept
    {
        return rfind(basic_string_view(std::addressof(c), 1), pos);
    }
    std::size_t rfind(const CharType *s, std::size_t pos, std::size_t count) const noexcept
    {
        return rfind(basic_string_view(s, count), pos);
    }
    std::size_t rfind(const CharType *s, std::size_t pos = npos) const
    {
        return rfind(basic_string_view(s), pos);
    }
    std::size_t find_first_of(basic_string_view v, std::size_t pos = 0) const noexcept
    {
        for(; pos < stringSize; pos++)
        {
            if(v.find(stringPointer[pos]) != npos)
                return pos;
        }
        return npos;
    }
    std::size_t find_first_of(CharType v, std::size_t pos = 0) const noexcept
    {
        return find(v, pos);
    }
    std::size_t find_first_of(const CharType *s, std::size_t pos, std::size_t count) const noexcept
    {
        return find_first_of(basic_string_view(s, count), pos);
    }
    std::size_t find_first_of(const CharType *s, std::size_t pos = 0) const
    {
        return find_first_of(basic_string_view(s), pos);
    }
    std::size_t find_first_not_of(basic_string_view v, std::size_t pos = 0) const noexcept
    {
        for(; pos < stringSize; pos++)
        {
            if(v.find(stringPointer[pos]) == npos)
                return pos;
        }
        return npos;
    }
    std::size_t find_first_not_of(CharType v, std::size_t pos = 0) const noexcept
    {
        return find_first_not_of(basic_string_view(std::addressof(v), 1), pos);
    }
    std::size_t find_first_not_of(const CharType *s, std::size_t pos, std::size_t count) const
        noexcept
    {
        return find_first_not_of(basic_string_view(s, count), pos);
    }
    std::size_t find_first_not_of(const CharType *s, std::size_t pos = 0) const
    {
        return find_first_not_of(basic_string_view(s), pos);
    }
    std::size_t find_last_of(basic_string_view v, std::size_t pos = npos) const noexcept
    {
        if(empty())
            return npos;
        pos = constexprMin(pos, stringSize - 1);
        for(std::size_t i = 0, count = pos; i < count; i++, pos--)
        {
            if(v.find(stringPointer[pos]) != npos)
                return pos;
        }
        return npos;
    }
    std::size_t find_last_of(CharType v, std::size_t pos = npos) const noexcept
    {
        return rfind(v, pos);
    }
    std::size_t find_last_of(const CharType *s, std::size_t pos, std::size_t count) const noexcept
    {
        return find_last_of(basic_string_view(s, count), pos);
    }
    std::size_t find_last_of(const CharType *s, std::size_t pos = npos) const
    {
        return find_last_of(basic_string_view(s), pos);
    }
    std::size_t find_last_not_of(basic_string_view v, std::size_t pos = npos) const noexcept
    {
        if(empty())
            return npos;
        pos = constexprMin(pos, stringSize - 1);
        for(std::size_t i = 0, count = pos; i < count; i++, pos--)
        {
            if(v.find(stringPointer[pos]) == npos)
                return pos;
        }
        return npos;
    }
    std::size_t find_last_not_of(CharType v, std::size_t pos = npos) const noexcept
    {
        return find_last_not_of(basic_string_view(std::addressof(v), 1), pos);
    }
    std::size_t find_last_not_of(const CharType *s, std::size_t pos, std::size_t count) const
        noexcept
    {
        return find_last_not_of(basic_string_view(s, count), pos);
    }
    std::size_t find_last_not_of(const CharType *s, std::size_t pos = npos) const
    {
        return find_last_not_of(basic_string_view(s), pos);
    }
    template <typename Allocator>
    explicit operator std::basic_string<CharType, TraitsType, Allocator>() const
    {
        return std::basic_string<CharType, TraitsType, Allocator>(stringPointer, stringSize);
    }
};

template <typename CharType, typename TraitsType>
constexpr std::size_t basic_string_view<CharType, TraitsType>::npos;

template <typename CharType, typename TraitsType>
constexpr bool operator==(basic_string_view<CharType, TraitsType> a,
                          basic_string_view<CharType, TraitsType> b) noexcept
{
    return a.size() == b.size() && TraitsType::compare(a.data(), b.data(), a.size()) == 0;
}

template <typename CharType, typename TraitsType>
constexpr bool operator!=(basic_string_view<CharType, TraitsType> a,
                          basic_string_view<CharType, TraitsType> b) noexcept
{
    return !operator==(a, b);
}

template <typename CharType, typename TraitsType>
constexpr bool operator<=(basic_string_view<CharType, TraitsType> a,
                          basic_string_view<CharType, TraitsType> b) noexcept
{
    return a.compare(b) <= 0;
}

template <typename CharType, typename TraitsType>
constexpr bool operator>=(basic_string_view<CharType, TraitsType> a,
                          basic_string_view<CharType, TraitsType> b) noexcept
{
    return a.compare(b) >= 0;
}

template <typename CharType, typename TraitsType>
constexpr bool operator<(basic_string_view<CharType, TraitsType> a,
                         basic_string_view<CharType, TraitsType> b) noexcept
{
    return a.compare(b) < 0;
}

template <typename CharType, typename TraitsType>
constexpr bool operator>(basic_string_view<CharType, TraitsType> a,
                         basic_string_view<CharType, TraitsType> b) noexcept
{
    return a.compare(b) > 0;
}

#define QUICK_SHELL_UTIL_STRING_VIEW_GENERATE_EXTRA_COMPARE_OPERATORS_NO_ALLOCATOR(...) \
    template <typename CharType, typename TraitsType>                                   \
    bool operator==(__VA_ARGS__) noexcept                                               \
    {                                                                                   \
        return operator==(static_cast<basic_string_view<CharType, TraitsType>>(a),      \
                          static_cast<basic_string_view<CharType, TraitsType>>(b));     \
    }                                                                                   \
                                                                                        \
    template <typename CharType, typename TraitsType>                                   \
    bool operator!=(__VA_ARGS__) noexcept                                               \
    {                                                                                   \
        return operator!=(static_cast<basic_string_view<CharType, TraitsType>>(a),      \
                          static_cast<basic_string_view<CharType, TraitsType>>(b));     \
    }                                                                                   \
                                                                                        \
    template <typename CharType, typename TraitsType>                                   \
    bool operator<=(__VA_ARGS__) noexcept                                               \
    {                                                                                   \
        return operator<=(static_cast<basic_string_view<CharType, TraitsType>>(a),      \
                          static_cast<basic_string_view<CharType, TraitsType>>(b));     \
    }                                                                                   \
                                                                                        \
    template <typename CharType, typename TraitsType>                                   \
    bool operator>=(__VA_ARGS__) noexcept                                               \
    {                                                                                   \
        return operator>=(static_cast<basic_string_view<CharType, TraitsType>>(a),      \
                          static_cast<basic_string_view<CharType, TraitsType>>(b));     \
    }                                                                                   \
                                                                                        \
    template <typename CharType, typename TraitsType>                                   \
    bool operator<(__VA_ARGS__) noexcept                                                \
    {                                                                                   \
        return operator<(static_cast<basic_string_view<CharType, TraitsType>>(a),       \
                         static_cast<basic_string_view<CharType, TraitsType>>(b));      \
    }                                                                                   \
                                                                                        \
    template <typename CharType, typename TraitsType>                                   \
    bool operator>(__VA_ARGS__) noexcept                                                \
    {                                                                                   \
        return operator>(static_cast<basic_string_view<CharType, TraitsType>>(a),       \
                         static_cast<basic_string_view<CharType, TraitsType>>(b));      \
    }

#define QUICK_SHELL_UTIL_STRING_VIEW_GENERATE_EXTRA_COMPARE_OPERATORS_WITH_ALLOCATOR(...) \
    template <typename CharType, typename TraitsType, typename Allocator>                 \
    bool operator==(__VA_ARGS__) noexcept                                                 \
    {                                                                                     \
        return operator==(static_cast<basic_string_view<CharType, TraitsType>>(a),        \
                          static_cast<basic_string_view<CharType, TraitsType>>(b));       \
    }                                                                                     \
                                                                                          \
    template <typename CharType, typename TraitsType, typename Allocator>                 \
    bool operator!=(__VA_ARGS__) noexcept                                                 \
    {                                                                                     \
        return operator!=(static_cast<basic_string_view<CharType, TraitsType>>(a),        \
                          static_cast<basic_string_view<CharType, TraitsType>>(b));       \
    }                                                                                     \
                                                                                          \
    template <typename CharType, typename TraitsType, typename Allocator>                 \
    bool operator<=(__VA_ARGS__) noexcept                                                 \
    {                                                                                     \
        return operator<=(static_cast<basic_string_view<CharType, TraitsType>>(a),        \
                          static_cast<basic_string_view<CharType, TraitsType>>(b));       \
    }                                                                                     \
                                                                                          \
    template <typename CharType, typename TraitsType, typename Allocator>                 \
    bool operator>=(__VA_ARGS__) noexcept                                                 \
    {                                                                                     \
        return operator>=(static_cast<basic_string_view<CharType, TraitsType>>(a),        \
                          static_cast<basic_string_view<CharType, TraitsType>>(b));       \
    }                                                                                     \
                                                                                          \
    template <typename CharType, typename TraitsType, typename Allocator>                 \
    bool operator<(__VA_ARGS__) noexcept                                                  \
    {                                                                                     \
        return operator<(static_cast<basic_string_view<CharType, TraitsType>>(a),         \
                         static_cast<basic_string_view<CharType, TraitsType>>(b));        \
    }                                                                                     \
                                                                                          \
    template <typename CharType, typename TraitsType, typename Allocator>                 \
    bool operator>(__VA_ARGS__) noexcept                                                  \
    {                                                                                     \
        return operator>(static_cast<basic_string_view<CharType, TraitsType>>(a),         \
                         static_cast<basic_string_view<CharType, TraitsType>>(b));        \
    }

QUICK_SHELL_UTIL_STRING_VIEW_GENERATE_EXTRA_COMPARE_OPERATORS_NO_ALLOCATOR(
    const CharType *a, basic_string_view<CharType, TraitsType> b)
QUICK_SHELL_UTIL_STRING_VIEW_GENERATE_EXTRA_COMPARE_OPERATORS_NO_ALLOCATOR(
    basic_string_view<CharType, TraitsType> a, const CharType *b)
QUICK_SHELL_UTIL_STRING_VIEW_GENERATE_EXTRA_COMPARE_OPERATORS_WITH_ALLOCATOR(
    basic_string_view<CharType, TraitsType> a, std::basic_string<CharType, TraitsType, Allocator> b)
QUICK_SHELL_UTIL_STRING_VIEW_GENERATE_EXTRA_COMPARE_OPERATORS_WITH_ALLOCATOR(
    std::basic_string<CharType, TraitsType, Allocator> a, basic_string_view<CharType, TraitsType> b)
#undef QUICK_SHELL_UTIL_STRING_VIEW_GENERATE_EXTRA_COMPARE_OPERATORS_NO_ALLOCATOR
#undef QUICK_SHELL_UTIL_STRING_VIEW_GENERATE_EXTRA_COMPARE_OPERATORS_WITH_ALLOCATOR

template <typename CharType, typename TraitsType>
std::basic_ostream<CharType, TraitsType> &operator<<(std::basic_ostream<CharType, TraitsType> &os,
                                                     basic_string_view<CharType, TraitsType> v)
{
    os << static_cast<std::basic_string<CharType, TraitsType>>(v);
    return os;
}

typedef basic_string_view<char> string_view;
typedef basic_string_view<wchar_t> wstring_view;
typedef basic_string_view<char16_t> u16string_view;
typedef basic_string_view<char32_t> u32string_view;

inline namespace literals
{
inline namespace string_view_literals
{
constexpr string_view operator"" _sv(const char *str, std::size_t length) noexcept
{
    return string_view(str, length);
}
constexpr wstring_view operator"" _sv(const wchar_t *str, std::size_t length) noexcept
{
    return wstring_view(str, length);
}
constexpr u16string_view operator"" _sv(const char16_t *str, std::size_t length) noexcept
{
    return u16string_view(str, length);
}
constexpr u32string_view operator"" _sv(const char32_t *str, std::size_t length) noexcept
{
    return u32string_view(str, length);
}
}
}
}
}

namespace std
{
template <typename CharType, typename TraitsType>
struct hash<quick_shell::util::basic_string_view<CharType, TraitsType>>
{
    std::size_t operator()(quick_shell::util::basic_string_view<CharType, TraitsType> v) const
    {
        typedef std::basic_string<CharType, TraitsType> stringType;
        return std::hash<stringType>(static_cast<stringType>(v));
    }
};
}

#endif /* UTIL_STRING_VIEW_H_ */
