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
#ifndef UTIL_ITEM_STREAM_H_
#define UTIL_ITEM_STREAM_H_

#include <type_traits>
#include <utility>

namespace quick_shell
{
namespace util
{
template <typename ItemStreamType>
struct ItemStreamTraits final
{
    static_assert(std::is_class<ItemStreamType>::value || std::is_union<ItemStreamType>::value, "");
    static_assert(std::is_nothrow_default_constructible<ItemStreamType>::value, "");
    static_assert(std::is_nothrow_destructible<ItemStreamType>::value, "");
    static_assert(std::is_nothrow_move_constructible<ItemStreamType>::value, "");
    static_assert(std::is_nothrow_move_assignable<ItemStreamType>::value, "");
    static_assert(std::is_copy_constructible<ItemStreamType>::value, "");
    static_assert(std::is_copy_assignable<ItemStreamType>::value, "");
    typedef decltype(std::declval<ItemStreamType &>().get()) GetReturnType;
    typedef decltype(std::declval<const ItemStreamType &>().peek()) PeekReturnType;
    typedef typename std::decay<GetReturnType>::type ItemType;
    static_assert(std::is_nothrow_default_constructible<ItemType>::value, "");
    static_assert(std::is_nothrow_destructible<ItemType>::value, "");
    static_assert(std::is_move_constructible<ItemType>::value, "");
    static_assert(std::is_move_assignable<ItemType>::value, "");
    static_assert(std::is_copy_constructible<ItemType>::value, "");
    static_assert(std::is_copy_assignable<ItemType>::value, "");
    static_assert(std::is_same<ItemType, typename std::decay<PeekReturnType>::type>::value, "");
    static_assert(
        std::is_same<bool, decltype(std::declval<const ItemStreamType &>().isAtEnd())>::value, "");
    static_assert(std::is_convertible<PeekReturnType, GetReturnType>::value, "");
    static constexpr GetReturnType get(ItemStreamType &itemStream) noexcept(
        noexcept(itemStream.get()))
    {
        return itemStream.get();
    }
    static constexpr PeekReturnType peek(const ItemStreamType &itemStream) noexcept(
        noexcept(itemStream.peek()))
    {
        return itemStream.peek();
    }
    static constexpr bool isAtEnd(const ItemStreamType &itemStream) noexcept(
        noexcept(itemStream.isAtEnd()))
    {
        return itemStream.isAtEnd();
    }
};

template <typename ItemStreamType>
class ItemStreamIterator final
{
private:
    ItemStreamType itemStream;
    typedef ItemStreamTraits<ItemStreamType> Traits;
    mutable typename Traits::ItemType item;
    mutable bool isItemValid;

public:
#error finish
#error add iterator value_type ... typedefs
public:
    constexpr ItemStreamIterator() noexcept : itemStream(), item(), isItemValid(false)
    {
    }
    constexpr explicit ItemStreamIterator(ItemStreamType &&itemStream) noexcept
        : itemStream(std::move(itemStream)),
          item(),
          isItemValid(false)
    {
    }
    constexpr explicit ItemStreamIterator(const ItemStreamType &itemStream) noexcept(
        std::is_nothrow_copy_constructible<ItemStreamType>::value)
        : itemStream(itemStream), item(), isItemValid(false)
    {
    }
    constexpr ItemStreamIterator begin() const
    {
        return *this;
    }
    constexpr ItemStreamIterator end() const noexcept
    {
        return ItemStreamIterator();
    }
    constexpr
};
}
}

#endif /* UTIL_ITEM_STREAM_H_ */
