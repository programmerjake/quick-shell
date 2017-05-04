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
#ifndef INPUT_TEXT_INPUT_H_
#define INPUT_TEXT_INPUT_H_

#include <string>
#include <utility>
#include <memory>
#include <vector>
#include <set>
#include <type_traits>
#include <cassert>
#include <iosfwd>
#include <algorithm>
#include "location.h"

namespace quick_shell
{
namespace input
{
constexpr int eof = std::char_traits<char>::eof();

struct TextInputStyle final
{
    static constexpr std::size_t defaultTabSize = 8;
    std::size_t tabSize;
    bool allowCRLFAsNewLine;
    bool allowCRAsNewLine;
    bool allowLFAsNewLine;
    constexpr explicit TextInputStyle(std::size_t tabSize,
                                      bool allowCRLFAsNewLine = false,
                                      bool allowCRAsNewLine = false,
                                      bool allowLFAsNewLine = true) noexcept
        : tabSize(tabSize),
          allowCRLFAsNewLine(allowCRLFAsNewLine),
          allowCRAsNewLine(allowCRAsNewLine),
          allowLFAsNewLine(allowLFAsNewLine)
    {
    }
    constexpr TextInputStyle() noexcept : TextInputStyle(defaultTabSize)
    {
    }
    constexpr bool operator==(const TextInputStyle &rt) const noexcept
    {
        return tabSize == rt.tabSize && allowCRLFAsNewLine == rt.allowCRLFAsNewLine
               && allowCRAsNewLine == rt.allowCRAsNewLine
               && allowLFAsNewLine == rt.allowLFAsNewLine;
    }
    constexpr bool operator!=(const TextInputStyle &rt) const noexcept
    {
        return !operator==(rt);
    }
};

constexpr bool isNewLine(int ch, const TextInputStyle &inputStyle) noexcept
{
    return (inputStyle.allowCRAsNewLine ? ch == '\r' : false)
           || (inputStyle.allowLFAsNewLine ? ch == '\n' : false);
}

constexpr bool isNewLinePair(int ch1, int ch2, const TextInputStyle &inputStyle) noexcept
{
    return inputStyle.allowCRLFAsNewLine ? ch1 == '\r' && ch2 == '\n' : false;
}

constexpr std::size_t getColumnAfterTab(std::size_t column,
                                        const TextInputStyle &inputStyle) noexcept
{
    return inputStyle.tabSize == 0 || column == 0 ?
               column + 1 :
               column + (inputStyle.tabSize - (column - 1) % inputStyle.tabSize);
}

struct LineAndColumn final
{
    std::size_t line;
    std::size_t column;
    constexpr LineAndColumn() noexcept : line(), column()
    {
    }
    constexpr explicit LineAndColumn(std::size_t line) noexcept : line(line), column()
    {
    }
    constexpr LineAndColumn(std::size_t line, std::size_t column) noexcept : line(line),
                                                                             column(column)
    {
    }
    friend std::ostream &operator<<(std::ostream &os, const LineAndColumn &v);
};

struct LineAndIndex final
{
    std::size_t line;
    std::size_t index;
    constexpr LineAndIndex() noexcept : line(), index()
    {
    }
    constexpr LineAndIndex(std::size_t line, std::size_t index) noexcept : line(line), index(index)
    {
    }
};

class TextInput
{
    TextInput(const TextInput &) = delete;
    TextInput &operator=(const TextInput &) = delete;

public:
    /** number of character positions an EOF takes */
    static constexpr std::size_t eofSize = 1;

protected:
    virtual std::size_t read(std::size_t startIndex,
                             unsigned char *buffer,
                             std::size_t bufferSize) = 0;

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
    TextInputStyle inputStyle;
    std::string name;
    std::vector<Chunk> chunks;
    std::size_t validMemorySize;
    std::set<std::size_t> eofPositions;

    /** doesn't have first line to save memory */
    std::vector<std::size_t> lineStartIndexes;

    /** index where all line start indexes before are in lineStartIndexes */
    std::size_t validLineStartIndexesIndex;

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
        std::size_t retval = getNextEOF(index);
        if(retval > validMemorySize)
            retval = validMemorySize;
        std::size_t nextChunkStartIndex = (index / chunkSize) * chunkSize + chunkSize;
        if(retval > nextChunkStartIndex)
            retval = nextChunkStartIndex;
        if(retval < index)
            retval = index;
        return retval;
    }
    unsigned char &readNonspecial(std::size_t index) const noexcept
    {
        assert(index < validMemorySize);
        return chunks[index / chunkSize][index % chunkSize];
    }
    void readTo(std::size_t targetIndex);
    void updateLineStartIndexes();
    template <typename Fn>
    void updateLineStartIndexesHelper(Fn &&fn);

public:
    explicit TextInput(std::string name,
                       const TextInputStyle &inputStyle,
                       std::shared_ptr<const unsigned char> memory,
                       std::size_t memorySize,
                       std::set<std::size_t> eofPositions,
                       bool retryAfterEOF)
        : retryAfterEOF(retryAfterEOF),
          inputStyle(inputStyle),
          name(std::move(name)),
          chunks(),
          validMemorySize(memorySize),
          eofPositions(std::move(eofPositions)),
          lineStartIndexes(),
          validLineStartIndexesIndex()
    {
        assert(memorySize == 0 || memory != nullptr);
        chunks.reserve((memorySize + chunkSize - 1) / chunkSize);
        for(std::size_t i = 0; i + 1 < memorySize / chunkSize; i++)
        {
            chunks.push_back(Chunk(std::shared_ptr<unsigned char>(
                memory, const_cast<unsigned char *>(memory.get() + i * chunkSize))));
        }
        std::size_t sizeLeft = memorySize % chunkSize;
        if(sizeLeft)
        {
            auto lastChunk = Chunk(AllocateTag{});
            const unsigned char *source = memory.get() + chunks.size() * chunkSize;
            for(std::size_t i = 0; i < sizeLeft; i++)
                lastChunk[i] = source[i];
            chunks.push_back(std::move(lastChunk));
        }
    }
    explicit TextInput(std::string name,
                       const TextInputStyle &inputStyle,
                       const unsigned char *memory,
                       std::size_t memorySize,
                       std::set<std::size_t> eofPositions,
                       bool retryAfterEOF)
        : retryAfterEOF(retryAfterEOF),
          inputStyle(inputStyle),
          name(std::move(name)),
          chunks(),
          validMemorySize(memorySize),
          eofPositions(std::move(eofPositions)),
          lineStartIndexes(),
          validLineStartIndexesIndex()
    {
        assert(memorySize == 0 || memory != nullptr);
        chunks.reserve((memorySize + chunkSize - 1) / chunkSize);
        for(std::size_t i = 0; i + 1 < memorySize / chunkSize; i++)
        {
            auto chunk = Chunk(AllocateTag{});
            const unsigned char *source = memory + i * chunkSize;
            for(std::size_t j = 0; j < chunkSize; j++)
                chunk[i] = source[i];
            chunks.push_back(std::move(chunk));
        }
        std::size_t sizeLeft = memorySize % chunkSize;
        if(sizeLeft)
        {
            auto lastChunk = Chunk(AllocateTag{});
            const unsigned char *source = memory + chunks.size() * chunkSize;
            for(std::size_t i = 0; i < sizeLeft; i++)
                lastChunk[i] = source[i];
            chunks.push_back(std::move(lastChunk));
        }
    }
    explicit TextInput(std::string name, const TextInputStyle &inputStyle, bool retryAfterEOF)
        : TextInput(std::move(name), inputStyle, nullptr, 0, std::set<std::size_t>(), retryAfterEOF)
    {
    }
    virtual ~TextInput() = default;
    const std::string &getName() const noexcept
    {
        return name;
    }
    void setName(std::string &&newName)
    {
        name.swap(newName);
    }
    void setName(const std::string &newName)
    {
        name = newName;
    }
    const TextInputStyle &getInputStyle() const noexcept
    {
        return inputStyle;
    }
    void setInputStyle(const TextInputStyle &newInputStyle)
    {
        if(inputStyle == newInputStyle)
            return;
        inputStyle = newInputStyle;
        lineStartIndexes.clear();
        validLineStartIndexesIndex = 0;
    }
    LineAndIndex getLineAndStartIndex(std::size_t index)
    {
        if(index >= validMemorySize)
        {
            if(!retryAfterEOF && !eofPositions.empty() && index >= *eofPositions.begin())
            {
                updateLineStartIndexes();
                std::size_t lastLineStart = lineStartIndexes.empty() ? 0 : lineStartIndexes.back();
                return LineAndIndex(lineStartIndexes.size() + 1 + index - lastLineStart, index);
            }
            readTo(index);
            if(index >= validMemorySize)
            {
                updateLineStartIndexes();
                std::size_t lastLineStart = lineStartIndexes.empty() ? 0 : lineStartIndexes.back();
                return LineAndIndex(lineStartIndexes.size() + 1 + index - lastLineStart, index);
            }
        }
        if(index >= validLineStartIndexesIndex)
            updateLineStartIndexes();
        std::size_t line =
            1 + lineStartIndexes.size()
            + (lineStartIndexes.rbegin() - std::lower_bound(lineStartIndexes.rbegin(),
                                                            lineStartIndexes.rend(),
                                                            index,
                                                            std::greater<std::size_t>()));
        return LineAndIndex(line, line <= 1 ? 0 : lineStartIndexes[line - 2]);
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
        friend class TextInput;

    public:
        typedef std::ptrdiff_t difference_type;
        typedef int value_type;
        typedef const int *pointer;
        typedef const int &reference;
        typedef std::input_iterator_tag iterator_category;

    private:
        static constexpr int invalidValue = (eof == -1 ? -2 : -1);

    private:
        TextInput *input;
        std::size_t index;
        const unsigned char *rangeCurrent;
        const unsigned char *rangeEnd;
        int value;

    private:
        void getValue()
        {
            if(rangeCurrent != rangeEnd)
                value = *rangeCurrent;
            else
                value = input->operator[](index);
        }

    private:
        Iterator(TextInput *input, std::size_t index)
            : input(input), index(index), rangeCurrent(), rangeEnd(), value(invalidValue)
        {
            std::size_t nextSpecialIndex = input->getNextSpecialIndex(index);
            if(nextSpecialIndex > index)
            {
                rangeCurrent = &input->readNonspecial(index);
                rangeEnd = &input->readNonspecial(nextSpecialIndex - 1) + 1;
                value = *rangeCurrent;
            }
            else
            {
                rangeCurrent = nullptr;
                rangeEnd = nullptr;
            }
        }

    public:
        Iterator() noexcept : input(nullptr),
                              index(-1),
                              rangeCurrent(nullptr),
                              rangeEnd(nullptr),
                              value(eof)
        {
        }
        const int *operator->()
        {
            if(value == invalidValue)
                getValue();
            return &value;
        }
        const int &operator*()
        {
            if(value == invalidValue)
                getValue();
            return value;
        }
        Iterator &operator++()
        {
            assert(input);
            index++;
            if(rangeCurrent == rangeEnd || rangeCurrent == rangeEnd - 1)
                *this = Iterator(input, index);
            else
                value = *++rangeCurrent;
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
            return index == rt.index;
        }
        bool operator!=(const Iterator &rt) const noexcept
        {
            return index != rt.index;
        }
        std::size_t getIndex() const noexcept
        {
            return index;
        }
        Location getLocation() const noexcept
        {
            return Location(index, input);
        }
    };
    Iterator begin()
    {
        return Iterator(this, 0);
    }
    Iterator iteratorAt(std::size_t index)
    {
        return Iterator(this, index);
    }
    Iterator end()
    {
        return Iterator();
    }
    LineAndColumn getLineAndColumn(std::size_t index)
    {
        auto lineAndStartIndex = getLineAndStartIndex(index);
        std::size_t column = 1;
        for(auto iter = iteratorAt(lineAndStartIndex.index); iter.getIndex() < index; ++iter)
        {
            int ch = *iter;
            if(ch == '\t')
                column = getColumnAfterTab(column, inputStyle);
            else
                column++;
        }
        return LineAndColumn(lineAndStartIndex.line, column);
    }
    std::size_t getLine(std::size_t index)
    {
        return getLineAndStartIndex(index).line;
    }
    std::size_t getLineStartIndex(std::size_t index)
    {
        return getLineAndStartIndex(index).index;
    }
    std::size_t getColumn(std::size_t index)
    {
        return getLineAndColumn(index).column;
    }
    LineAndColumn getLineAndColumn(const Iterator &i)
    {
        return getLineAndColumn(i.getIndex());
    }
    LineAndIndex getLineAndStartIndex(const Iterator &i)
    {
        return getLineAndStartIndex(i.getIndex());
    }
    std::size_t getLine(const Iterator &i)
    {
        return getLine(i.getIndex());
    }
    std::size_t getLineStartIndex(const Iterator &i)
    {
        return getLineStartIndex(i.getIndex());
    }
    std::size_t getColumn(const Iterator &i)
    {
        return getColumn(i.getIndex());
    }
    Location getLocation(std::size_t index) noexcept
    {
        return Location(index, this);
    }
    Location getLocation(const Iterator &i) noexcept
    {
        return i.getLocation();
    }
    LocationSpan getLocationSpan(std::size_t beginIndex, std::size_t endIndex) noexcept
    {
        return LocationSpan(beginIndex, endIndex, this);
    }
};

class LineContinuationRemovingIterator final
{
public:
    typedef std::ptrdiff_t difference_type;
    typedef int value_type;
    typedef const int *pointer;
    typedef const int &reference;
    typedef std::input_iterator_tag iterator_category;

private:
    mutable TextInput::Iterator iter;
    mutable bool isAtValidLocation;
    void moveToValidLocation() const
    {
        auto textInputStyle = iter.getLocation().input->getInputStyle();
        while(*iter == '\\')
        {
            auto iter2 = iter;
            ++iter2;
            if(*iter2 == '\r')
            {
                if(textInputStyle.allowCRLFAsNewLine)
                {
                    auto iter3 = iter2;
                    ++iter3;
                    if(*iter3 == '\n')
                    {
                        iter = iter3;
                        ++iter;
                        continue;
                    }
                }
                if(textInputStyle.allowCRAsNewLine)
                {
                    iter = iter2;
                    ++iter;
                    continue;
                }
            }
            if(textInputStyle.allowLFAsNewLine && *iter2 == '\n')
            {
                iter = iter2;
                ++iter;
                continue;
            }
            break;
        }
        isAtValidLocation = true;
    }

public:
    explicit LineContinuationRemovingIterator(const TextInput::Iterator &iter) noexcept
        : iter(iter),
          isAtValidLocation(false)
    {
    }
    LineContinuationRemovingIterator() noexcept : iter(), isAtValidLocation(true)
    {
    }
    const int *operator->() const
    {
    	if(!isAtValidLocation)
    		moveToValidLocation();
        return iter.operator->();
    }
    const int &operator*() const
    {
    	if(!isAtValidLocation)
    		moveToValidLocation();
        return *iter;
    }
    LineContinuationRemovingIterator &operator++()
    {
    	if(!isAtValidLocation)
    		moveToValidLocation();
        ++iter;
        isAtValidLocation = false;
        return *this;
    }
    LineContinuationRemovingIterator operator++(int)
    {
        auto retval = *this;
        operator++();
        return retval;
    }
    bool operator==(const LineContinuationRemovingIterator &rt) const
    {
    	if(iter == rt.iter)
    		return true;
    	if(!isAtValidLocation)
    		moveToValidLocation();
    	if(!rt.isAtValidLocation)
    		rt.moveToValidLocation();
        return iter == rt.iter;
    }
    bool operator!=(const LineContinuationRemovingIterator &rt) const
    {
    	return !operator==(rt);
    }
    Location getLocation() const
    {
    	if(!isAtValidLocation)
    		moveToValidLocation();
        return iter.getLocation();
    }
    TextInput::Iterator getBaseIterator() const
    {
    	if(!isAtValidLocation)
    		moveToValidLocation();
        return iter;
    }
    explicit operator TextInput::Iterator() const
    {
    	if(!isAtValidLocation)
    		moveToValidLocation();
        return iter;
    }
};
}
}

#endif /* INPUT_TEXT_INPUT_H_ */
