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
#include "input.h"
#include <iostream>

namespace quick_shell
{
namespace input
{
constexpr std::size_t Input::eofSize;

Input::Chunk::Chunk(std::shared_ptr<unsigned char> memory) : memory(memory.get())
{
    freeArg = static_cast<void *>(new std::shared_ptr<unsigned char>(std::move(memory)));
    freeFn = [](void *freeArg)
    {
        delete static_cast<std::shared_ptr<unsigned char> *>(freeArg);
    };
}

void Input::readTo(std::size_t targetIndex)
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
}
}
