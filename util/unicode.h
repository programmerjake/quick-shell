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
#ifndef UTIL_UNICODE_H_
#define UTIL_UNICODE_H_

#include <string>
#include "string_view.h"

namespace quick_shell
{
namespace util
{
struct EncodedUTF8CodePoint final
{
    static constexpr std::size_t maxSize = 6; // for characters up to 0x80000000
    unsigned char bytes[maxSize];
    std::size_t bytesUsed;
    constexpr EncodedUTF8CodePoint() noexcept : bytes{}, bytesUsed(0)
    {
    }
    constexpr explicit EncodedUTF8CodePoint(unsigned char ch1) noexcept : bytes{ch1}, bytesUsed(1)
    {
    }
    constexpr EncodedUTF8CodePoint(unsigned char ch1, unsigned char ch2) noexcept : bytes{ch1, ch2},
                                                                                    bytesUsed(2)
    {
    }
    constexpr EncodedUTF8CodePoint(unsigned char ch1, unsigned char ch2, unsigned char ch3) noexcept
        : bytes{ch1, ch2, ch3},
          bytesUsed(3)
    {
    }
    constexpr EncodedUTF8CodePoint(unsigned char ch1,
                                   unsigned char ch2,
                                   unsigned char ch3,
                                   unsigned char ch4) noexcept : bytes{ch1, ch2, ch3, ch4},
                                                                 bytesUsed(4)
    {
    }
    constexpr EncodedUTF8CodePoint(unsigned char ch1,
                                   unsigned char ch2,
                                   unsigned char ch3,
                                   unsigned char ch4,
                                   unsigned char ch5) noexcept : bytes{ch1, ch2, ch3, ch4, ch5},
                                                                 bytesUsed(5)
    {
    }
    constexpr EncodedUTF8CodePoint(unsigned char ch1,
                                   unsigned char ch2,
                                   unsigned char ch3,
                                   unsigned char ch4,
                                   unsigned char ch5,
                                   unsigned char ch6) noexcept
        : bytes{ch1, ch2, ch3, ch4, ch5, ch6},
          bytesUsed(6)
    {
    }
    constexpr operator util::string_view() const noexcept
    {
        return util::string_view(reinterpret_cast<const char *>(bytes), bytesUsed);
    }
    explicit operator std::string() const
    {
        return static_cast<std::string>(operator util::string_view());
    }
};

constexpr EncodedUTF8CodePoint encodeUTF8(char32_t ch) noexcept
{
    return ch >= 0x80000000UL ? EncodedUTF8CodePoint(0) : ch >= 0x4000000UL ?
                                EncodedUTF8CodePoint(0xFC | (ch >> 30),
                                                     0x80 | ((ch >> 24) & 0x3F),
                                                     0x80 | ((ch >> 18) & 0x3F),
                                                     0x80 | ((ch >> 12) & 0x3F),
                                                     0x80 | ((ch >> 6) & 0x3F),
                                                     0x80 | (ch & 0x3F)) :
                                ch >= 0x200000UL ?
                                EncodedUTF8CodePoint(0xF8 | (ch >> 24),
                                                     0x80 | ((ch >> 18) & 0x3F),
                                                     0x80 | ((ch >> 12) & 0x3F),
                                                     0x80 | ((ch >> 6) & 0x3F),
                                                     0x80 | (ch & 0x3F)) :
                                ch >= 0x10000UL ?
                                EncodedUTF8CodePoint(0xF0 | (ch >> 18),
                                                     0x80 | ((ch >> 12) & 0x3F),
                                                     0x80 | ((ch >> 6) & 0x3F),
                                                     0x80 | (ch & 0x3F)) :
                                ch >= 0x800U ?
                                EncodedUTF8CodePoint(0xE0 | (ch >> 12),
                                                     0x80 | ((ch >> 6) & 0x3F),
                                                     0x80 | (ch & 0x3F)) :
                                ch >= 0x80U ?
                                EncodedUTF8CodePoint(0xC0 | (ch >> 6), 0x80 | (ch & 0x3F)) :
                                EncodedUTF8CodePoint(ch);
}

struct DecodeUTF8Helper final
{
    static constexpr int invalidByteValue = std::char_traits<char>::eof();
    static_assert(static_cast<int>(static_cast<unsigned char>(invalidByteValue))
                      != invalidByteValue,
                  "");
    static constexpr int getByteValue(int v) noexcept
    {
        return static_cast<int>(static_cast<unsigned char>(v)) == v ? v : invalidByteValue;
    }
    static constexpr int getByteValue(char v) noexcept
    {
        return static_cast<unsigned char>(v);
    }
    static constexpr int getByteValue(signed char v) noexcept
    {
        return static_cast<unsigned char>(v);
    }
    static constexpr int getByteValue(unsigned char v) noexcept
    {
        return v;
    }
};

constexpr std::char_traits<char32_t>::int_type decodeUTF8InvalidResult =
    std::char_traits<char32_t>::eof();

static_assert(decodeUTF8InvalidResult >= 0x80000000UL, "");

template <typename IteratorType,
          bool allowOutOfRangeCharacters = false,
          bool allowOverlongCharacters = false,
          bool allowDoubleByteNull = allowOverlongCharacters,
          bool allowSurrogates = allowOutOfRangeCharacters>
std::char_traits<char32_t>::int_type decodeUTF8(IteratorType &iter) noexcept(
    noexcept(++iter) && noexcept(DecodeUTF8Helper::getByteValue(*iter)))
{
    constexpr int eof = std::char_traits<char>::eof();
    int ch0 = DecodeUTF8Helper::getByteValue(*iter);
    if(ch0 == DecodeUTF8Helper::invalidByteValue || ch0 >= 0xFE || (ch0 & 0xC0) == 0x80)
        return decodeUTF8InvalidResult;
    if(!allowOutOfRangeCharacters && ch0 > 0xF4)
        return decodeUTF8InvalidResult;
    if(!allowDoubleByteNull && !allowOverlongCharacters && ch0 == 0xC0)
        return decodeUTF8InvalidResult;
    if(!allowOverlongCharacters && ch0 == 0xC1)
        return decodeUTF8InvalidResult;
    ++iter;
    if(ch0 < 0x80)
        return ch0;
    int ch1 = DecodeUTF8Helper::getByteValue(*iter);
    if(ch1 == DecodeUTF8Helper::invalidByteValue || (ch1 & 0xC0) != 0x80)
        return decodeUTF8InvalidResult;
    if(ch0 == 0xC0 && ch1 == 0x80)
    {
        if(allowDoubleByteNull)
        {
            ++iter;
            return 0;
        }
        return decodeUTF8InvalidResult;
    }
    if(!allowOverlongCharacters && ch0 == 0xE0 && ch1 < 0xA0)
        return decodeUTF8InvalidResult;
    if(!allowOverlongCharacters && ch0 == 0xF0 && ch1 < 0x90)
        return decodeUTF8InvalidResult;
    if(!allowOverlongCharacters && ch0 == 0xF8 && ch1 < 0x88)
        return decodeUTF8InvalidResult;
    if(!allowOverlongCharacters && ch0 == 0xFC && ch1 < 0x84)
        return decodeUTF8InvalidResult;
    if(!allowOutOfRangeCharacters && ch0 == 0xF4 && ch1 > 0x8F)
        return decodeUTF8InvalidResult;
    if(!allowSurrogates && ch0 == 0xED && ch1 > 0x9F)
        return decodeUTF8InvalidResult;
    ++iter;
    if(ch0 < 0xE0) // 2 byte
    {
        char32_t retval = ch0 & 0x1F;
        retval <<= 6;
        retval |= ch1 & 0x3F;
        return retval;
    }
    int ch2 = DecodeUTF8Helper::getByteValue(*iter);
    if(ch2 == DecodeUTF8Helper::invalidByteValue || (ch2 & 0xC0) != 0x80)
        return decodeUTF8InvalidResult;
    ++iter;
    if(ch0 < 0xF0) // 3 byte
    {
        char32_t retval = ch0 & 0xF;
        retval <<= 6;
        retval |= ch1 & 0x3F;
        retval <<= 6;
        retval |= ch2 & 0x3F;
        return retval;
    }
    int ch3 = DecodeUTF8Helper::getByteValue(*iter);
    if(ch3 == DecodeUTF8Helper::invalidByteValue || (ch3 & 0xC0) != 0x80)
        return decodeUTF8InvalidResult;
    ++iter;
    if(ch0 < 0xF8) // 4 byte
    {
        char32_t retval = ch0 & 0x7;
        retval <<= 6;
        retval |= ch1 & 0x3F;
        retval <<= 6;
        retval |= ch2 & 0x3F;
        retval <<= 6;
        retval |= ch3 & 0x3F;
        return retval;
    }
    int ch4 = DecodeUTF8Helper::getByteValue(*iter);
    if(ch4 == DecodeUTF8Helper::invalidByteValue || (ch4 & 0xC0) != 0x80)
        return decodeUTF8InvalidResult;
    ++iter;
    if(ch0 < 0xFC) // 5 byte
    {
        char32_t retval = ch0 & 0x3;
        retval <<= 6;
        retval |= ch1 & 0x3F;
        retval <<= 6;
        retval |= ch2 & 0x3F;
        retval <<= 6;
        retval |= ch3 & 0x3F;
        retval <<= 6;
        retval |= ch4 & 0x3F;
        return retval;
    }
    // 6 byte
    int ch5 = DecodeUTF8Helper::getByteValue(*iter);
    if(ch5 == DecodeUTF8Helper::invalidByteValue || (ch5 & 0xC0) != 0x80)
        return decodeUTF8InvalidResult;
    ++iter;
    char32_t retval = ch0 & 0x1;
    retval <<= 6;
    retval |= ch1 & 0x3F;
    retval <<= 6;
    retval |= ch2 & 0x3F;
    retval <<= 6;
    retval |= ch3 & 0x3F;
    retval <<= 6;
    retval |= ch4 & 0x3F;
    retval <<= 6;
    retval |= ch5 & 0x3F;
    return retval;
}
}
}

#endif /* UTIL_UNICODE_H_ */
