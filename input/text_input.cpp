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
#include <iostream>

#include "text_input.h"

namespace quick_shell
{
namespace input
{
constexpr std::size_t TextInputStyle::defaultTabSize;

std::ostream &operator<<(std::ostream &os, const LineAndColumn &v)
{
    os << v.line << ':' << v.column;
    return os;
}

constexpr std::size_t TextInput::eofSize;

TextInput::Chunk::Chunk(std::shared_ptr<unsigned char> memory) : memory(memory.get())
{
    freeArg = static_cast<void *>(new std::shared_ptr<unsigned char>(std::move(memory)));
    freeFn = [](void *freeArg)
    {
        delete static_cast<std::shared_ptr<unsigned char> *>(freeArg);
    };
}

void TextInput::readTo(std::size_t targetIndex)
{
#ifndef NDEBUG
    if(targetIndex > validMemorySize) // advancing by more than one
        std::cerr << "Input: debug: input skipped from index " << validMemorySize << " to "
                  << targetIndex << std::endl;
#endif
    while(targetIndex >= validMemorySize)
    {
        if(!retryAfterEOF && !eofPositions.empty())
            return;
        std::size_t chunkIndex = validMemorySize / chunkSize;
        if(chunkIndex >= chunks.size())
            chunks.push_back(Chunk(AllocateTag{}));
        assert(chunkIndex < chunks.size());
        auto &chunk = chunks[chunkIndex];
        assert(chunk);
        std::size_t startIndex = validMemorySize;
        std::size_t chunkStartIndex = startIndex % chunkSize;
        std::size_t spaceLeft = chunkSize - chunkStartIndex;
        unsigned char *memory = chunk.data() + chunkStartIndex;
        std::size_t readCount = read(startIndex, memory, spaceLeft);
        assert(readCount <= spaceLeft);
        if(readCount == 0)
        {
            eofPositions.insert(startIndex);
            *memory = '\0';
            validMemorySize++;
        }
        else
        {
            validMemorySize += readCount;
        }
    }
}

template <typename Fn>
void TextInput::updateLineStartIndexesHelper(Fn &&fn)
{
    std::size_t endIndex = validMemorySize;
    std::size_t startIndex = validLineStartIndexesIndex;
    assert(endIndex > startIndex);
    auto iter = iteratorAt(validLineStartIndexesIndex);
    for(std::size_t i = validLineStartIndexesIndex; i < validMemorySize; i++)
    {
        int ch = *iter;
        if(i + 1 < validMemorySize)
        {
            int ch2 = *++iter;
            if(isNewLinePair(ch, ch2, inputStyle))
            {
                fn(i + 2);
                continue;
            }
        }
        if(isNewLine(ch, inputStyle) || ch == eof)
            fn(i + 1);
    }
    assert(validMemorySize == endIndex); // verify that we haven't read any more
}

void TextInput::updateLineStartIndexes()
{
    if(validLineStartIndexesIndex < validMemorySize)
    {
        std::size_t lineStartIndexCount = lineStartIndexes.size();
        updateLineStartIndexesHelper([&](std::size_t index)
                                     {
                                         lineStartIndexCount++;
                                     });
        lineStartIndexes.reserve(lineStartIndexCount);
        updateLineStartIndexesHelper([&](std::size_t index)
                                     {
                                         lineStartIndexes.push_back(index);
                                     });
        validLineStartIndexesIndex = validMemorySize;
    }
}
}
}
