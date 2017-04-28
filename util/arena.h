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
#ifndef UTIL_ARENA_H_
#define UTIL_ARENA_H_

#include <vector>
#include <memory>
#include <utility>
#include <type_traits>
#include <functional>

namespace quick_shell
{
namespace util
{
class Arena;

template <typename T>
class ArenaPtr final
{
    friend class Arena;

private:
    T *ptr;

public:
    constexpr ArenaPtr() noexcept : ptr(nullptr)
    {
    }
    constexpr ArenaPtr(std::nullptr_t) noexcept : ptr(nullptr)
    {
    }

private:
    explicit ArenaPtr(T *ptr) noexcept : ptr(ptr)
    {
    }

public:
    template <typename T2,
              typename = typename std::enable_if<std::is_convertible<T2 *, T *>::value>::type>
    constexpr ArenaPtr(ArenaPtr<T2> v) noexcept : ptr(v.get())
    {
    }
    void reset() noexcept
    {
        ptr = nullptr;
    }
    void swap(ArenaPtr<T> &rt) noexcept
    {
        std::swap(ptr, rt.ptr);
    }
    constexpr T *get() const noexcept
    {
        return ptr;
    }
    constexpr explicit operator bool() const noexcept
    {
        return ptr != nullptr;
    }
    constexpr typename std::add_lvalue_reference<T>::type operator*() const noexcept
    {
        return *ptr;
    }
    constexpr T *operator->() const noexcept
    {
        return ptr;
    }
    constexpr explicit operator T *() const noexcept
    {
        return ptr;
    }
    template <typename To, typename From>
    friend constexpr ArenaPtr<To> static_pointer_cast(ArenaPtr<From> v) noexcept;
    template <typename To, typename From>
    friend constexpr ArenaPtr<To> dynamic_pointer_cast(ArenaPtr<From> v) noexcept;
    template <typename To, typename From>
    friend constexpr ArenaPtr<To> const_pointer_cast(ArenaPtr<From> v) noexcept;
};

template <typename T>
void swap(ArenaPtr<T> &a, ArenaPtr<T> &b) noexcept
{
    a.swap(b);
}

template <typename T1, typename T2>
constexpr bool operator==(ArenaPtr<T1> a, ArenaPtr<T2> b) noexcept
{
    return a.get() == b.get();
}

template <typename T1, typename T2>
constexpr bool operator!=(ArenaPtr<T1> a, ArenaPtr<T2> b) noexcept
{
    return !operator==(a, b);
}

template <typename T>
constexpr bool operator==(std::nullptr_t, ArenaPtr<T> v) noexcept
{
    return v.get() == nullptr;
}

template <typename T>
constexpr bool operator!=(std::nullptr_t, ArenaPtr<T> v) noexcept
{
    return v.get() != nullptr;
}

template <typename T>
constexpr bool operator==(ArenaPtr<T> v, std::nullptr_t) noexcept
{
    return v.get() == nullptr;
}

template <typename T>
constexpr bool operator!=(ArenaPtr<T> v, std::nullptr_t) noexcept
{
    return v.get() != nullptr;
}

template <typename T1, typename T2>
constexpr bool operator<(ArenaPtr<T1> a, ArenaPtr<T2> b) noexcept
{
    return std::less<typename std::common_type<T1 *, T2 *>::type>()(a.get(), b.get());
}

template <typename T1, typename T2>
constexpr bool operator>(ArenaPtr<T1> a, ArenaPtr<T2> b) noexcept
{
    return operator<(b, a);
}

template <typename T1, typename T2>
constexpr bool operator<=(ArenaPtr<T1> a, ArenaPtr<T2> b) noexcept
{
    return !operator<(b, a);
}

template <typename T1, typename T2>
constexpr bool operator>=(ArenaPtr<T1> a, ArenaPtr<T2> b) noexcept
{
    return !operator<(a, b);
}

template <typename To, typename From>
constexpr ArenaPtr<To> static_pointer_cast(ArenaPtr<From> v) noexcept
{
	return ArenaPtr<To>(static_cast<To *>(v.get()));
}

template <typename To, typename From>
constexpr ArenaPtr<To> dynamic_pointer_cast(ArenaPtr<From> v) noexcept
{
	return ArenaPtr<To>(dynamic_cast<To *>(v.get()));
}

template <typename To, typename From>
constexpr ArenaPtr<To> const_pointer_cast(ArenaPtr<From> v) noexcept
{
	return ArenaPtr<To>(const_cast<To *>(v.get()));
}

class Arena final
{
private:
    struct Allocation final
    {
        void *memory;
        void (*destroyFn)(void *memory);
        Allocation() noexcept : memory(), destroyFn()
        {
        }
        explicit Allocation(void *memory, void (*destroyFn)(void *memory)) noexcept
            : memory(memory), destroyFn(destroyFn)
        {
        }
        Allocation(Allocation &&rt) noexcept : memory(rt.memory), destroyFn(rt.destroyFn)
        {
            rt.memory = nullptr;
            rt.destroyFn = nullptr;
        }
        ~Allocation() noexcept
        {
            if(destroyFn)
                destroyFn(memory);
        }
        void clear() noexcept
        {
            Allocation().swap(*this);
        }
        void swap(Allocation &other) noexcept
        {
            std::swap(memory, other.memory);
            std::swap(destroyFn, other.destroyFn);
        }
        Allocation &operator=(Allocation rt) noexcept
        {
            swap(rt);
            return *this;
        }
    };

private:
    std::vector<Allocation> allocations;

private:
    void mergeHelper(Arena &other) noexcept
    {
        for(auto &allocation : other.allocations)
            allocations.push_back(std::move(allocation));
        other.allocations.clear();
    }

public:
    void merge(Arena &&other)
    {
        allocations.reserve(allocations.size() + other.allocations.size());
        mergeHelper(other);
    }
    template <typename T, typename... Args>
    ArenaPtr<T> allocate(Args &&... args)
    {
        allocations.emplace_back();
        try
        {
            auto *retval = new T(std::forward<Args>(args)...);
            allocations.back() =
                Allocation(const_cast<void *>(static_cast<const volatile void *>(retval)),
                           [](void *memory) noexcept {
                               auto *object = static_cast<T *>(memory);
                               delete object;
                           });
            return retval;
        }
        catch(...)
        {
            allocations.pop_back();
            throw;
        }
    }
};
}
}

namespace std
{
template <typename T>
struct hash<quick_shell::util::ArenaPtr<T>>
{
    std::size_t operator()(quick_shell::util::ArenaPtr<T> v) const
    {
        return std::hash<T *>()(v.get());
    }
};
}

#endif /* UTIL_ARENA_H_ */
