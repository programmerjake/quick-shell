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
#ifndef INPUT_INPUT_H_
#define INPUT_INPUT_H_

#include <string>
#include <utility>
#include <memory>
#include <vector>
#include <set>
#include <type_traits>
#include <cassert>

namespace quick_shell
{
namespace input
{
constexpr int eof = std::char_traits<char>::eof();

class Input
{
    Input(const Input &) = delete;
    Input &operator=(const Input &) = delete;

public:
    /** number of character positions an EOF takes */
    static constexpr std::size_t eofSize = 1;

protected:
    virtual std::size_t read(std::size_t startIndex, unsigned char *buffer, std::size_t count) = 0;

protected:
    const bool retryAfterEOF;

private:
    static constexpr std::size_t chunkSize = 4096;
    struct AllocateTag
    {
    };
    struct Chunk final
    {
        typedef void (*FreeFn)(void *arg);

    private:
        unsigned char *memory;
        void *freeArg;
        FreeFn freeFn;
        static void deleteFreeFn(void *memory) noexcept
        {
            delete[] static_cast<unsigned char *>(memory);
        }

    public:
        constexpr Chunk() noexcept : memory(nullptr), freeArg(nullptr), freeFn(nullptr)
        {
        }
        explicit Chunk(AllocateTag) : Chunk(new unsigned char[chunkSize])
        {
        }
        constexpr explicit Chunk(unsigned char *memory) noexcept : memory(memory),
                                                                   freeArg(memory),
                                                                   freeFn(deleteFreeFn)
        {
        }
        explicit Chunk(unsigned char *memory, FreeFn deleter) noexcept : memory(memory),
                                                                         freeArg(memory),
                                                                         freeFn(deleter)
        {
        }
        explicit Chunk(unsigned char *memory, void *freeArg, FreeFn deleter) noexcept
            : memory(memory),
              freeArg(freeArg),
              freeFn(deleter)
        {
        }
        explicit Chunk(std::shared_ptr<unsigned char> memory);
        Chunk(Chunk &&rt) noexcept : memory(rt.memory), freeArg(rt.freeArg), freeFn(rt.freeFn)
        {
            rt.memory = nullptr;
            rt.freeArg = nullptr;
            rt.freeFn = nullptr;
        }
        ~Chunk()
        {
            if(freeFn)
                freeFn(freeArg);
        }
        void swap(Chunk &other) noexcept
        {
            using std::swap;
            swap(memory, other.memory);
            swap(freeArg, other.freeArg);
            swap(freeFn, other.freeFn);
        }
        Chunk &operator=(Chunk rt) noexcept
        {
            swap(rt);
            return *this;
        }
        unsigned char *data() const noexcept
        {
            return memory;
        }
        static constexpr std::size_t size() noexcept
        {
            return chunkSize;
        }
        unsigned char &operator[](std::size_t index) const noexcept
        {
            assert(index < chunkSize);
            assert(memory);
            return memory[index];
        }
        explicit operator bool() const noexcept
        {
            return memory != nullptr;
        }
    };

private:
    std::string name;
    std::vector<Chunk> chunks;
    std::size_t validMemorySize;
    std::set<std::size_t> eofPositions;

private:
    std::size_t getNextEOF(std::size_t index) const
    {
        auto iter = eofPositions.lower_bound(index);
        if(iter == eofPositions.end())
            return -1;
        return *iter;
    }
    std::size_t getNextSpecialIndex(std::size_t index) const
    {
        if(index >= validMemorySize)
            return index;
        std::size_t nextEOF = getNextEOF(index);
        if(nextEOF < validMemorySize)
            return nextEOF;
        return validMemorySize;
    }
    unsigned char readNonspecial(std::size_t index) const noexcept
    {
        assert(index < validMemorySize);
        return chunks[index / chunkSize][index % chunkSize];
    }
    void readTo(std::size_t targetIndex);

public:
    const std::string &getName() const noexcept
    {
        return name;
    }
    virtual ~Input() = default;
    explicit Input(std::string name, bool retryAfterEOF = false)
        : retryAfterEOF(retryAfterEOF),
          name(std::move(name)),
          chunks(),
          validMemorySize(0),
          eofPositions()
    {
    }
    explicit Input(std::string name,
                   std::shared_ptr<const unsigned char> memory,
                   std::size_t memorySize,
                   std::set<std::size_t> eofPositions,
                   bool retryAfterEOF = true)
        : retryAfterEOF(retryAfterEOF),
          name(std::move(name)),
          chunks(),
          validMemorySize(memorySize),
          eofPositions(std::move(eofPositions))
    {
        assert(memorySize == 0 || memory != nullptr);
        chunks.reserve((memorySize + chunkSize - 1) / memorySize);
        for(std::size_t i = 0; i + 1 < memorySize / chunkSize; i++)
        {
            chunks.push_back(Chunk(std::shared_ptr<unsigned char>(
                memory, const_cast<unsigned char *>(memory.get() + i * chunkSize))));
        }
        chunks.push_back(Chunk(AllocateTag{}));
    }
    int operator[](std::size_t index)
    {
        if(index >= validMemorySize)
        {
            if(!retryAfterEOF && !eofPositions.empty() && index >= *eofPositions.begin())
                return eof;
            readTo(index);
            if(index >= validMemorySize)
                return eof;
        }
        if(eofPositions.count(index) != 0)
            return eof;
        return readNonspecial(index);
    }
    class Iterator final
    {
    public:
        typedef std::ptrdiff_t difference_type;
        typedef int value_type;
        typedef const int *pointer;
        typedef const int &reference;
        typedef std::input_iterator_tag iterator_category;

    private:
        friend class Input;
        Input *input;
        std::size_t nextSpecialIndexAfter;
        std::size_t index;
        int value;

    private:
        Iterator(Input *input, std::size_t index)
            : input(input), nextSpecialIndexAfter(), index(index), value(input->operator[](index))
        {
            if(value == eof && !input->retryAfterEOF)
            {
                input = nullptr;
                index = 0;
            }
            else
            {
                nextSpecialIndexAfter = input->getNextSpecialIndex(index + 1);
            }
        }

    public:
        Iterator() noexcept : input(nullptr), nextSpecialIndexAfter(), index(1), value(eof)
        {
        }
        const int *operator->() const noexcept
        {
            return &value;
        }
        const int &operator*() const noexcept
        {
            return value;
        }
        Iterator &operator++()
        {
            if(!input)
            {
                index = 1;
                return *this;
            }
            index++;
            assert(index != static_cast<std::size_t>(-1));
            if(index == nextSpecialIndexAfter)
                *this = Iterator(input, index);
            return *this;
        }
        Iterator operator++(int)
        {
            auto retval = *this;
            operator++();
            return retval;
        }
        bool operator==(const Iterator &rt) const noexcept
        {
            return input == rt.input && index == rt.index;
        }
        bool operator!=(const Iterator &rt) const noexcept
        {
            return input != rt.input && index != rt.index;
        }
    };
    Iterator begin()
    {
        return Iterator(this, 0);
    }
    Iterator end()
    {
        return Iterator();
    }
};
}
}

#endif /* INPUT_INPUT_H_ */
