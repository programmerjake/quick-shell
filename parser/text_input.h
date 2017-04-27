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
#ifndef PARSER_TEXT_INPUT_H_
#define PARSER_TEXT_INPUT_H_

#include <vector>
#include <string>
#include <utility>
#include <fstream>
#include <cstdio>
#include "../util/string_view.h"
#include "location.h"

namespace quick_shell
{
namespace parser
{
class TextInput
{
    TextInput(const TextInput &) = delete;
    TextInput &operator=(const TextInput &) = delete;

private:
    std::vector<char> inputBuffer;
    std::size_t inputPosition = 0;
    bool gotEOF = false;

protected:
    virtual std::vector<char> fillInputBuffer(std::vector<char> inputBuffer)
    {
        // inputBuffer is cleared before call
        // return empty buffer to signify EOF
        return inputBuffer;
    }

public:
    Location location;
    std::size_t tabSize = Location::defaultTabSize;

public:
    virtual ~TextInput() = default;
    TextInput() : inputBuffer(), location()
    {
    }
    explicit TextInput(const Location &location) : inputBuffer(), location(location)
    {
    }
    explicit TextInput(std::vector<char> inputBuffer, const Location &location = Location())
        : inputBuffer(std::move(inputBuffer)), location(location)
    {
    }
    explicit TextInput(util::string_view inputBuffer, const Location &location = Location())
        : inputBuffer(inputBuffer.begin(), inputBuffer.end()), location(location)
    {
    }
    static constexpr int eof = std::char_traits<char>::eof();
    int peek()
    {
        if(gotEOF)
            return eof;
        if(inputPosition >= inputBuffer.size())
        {
            inputPosition = 0;
            inputBuffer.clear();
            inputBuffer = fillInputBuffer(std::move(inputBuffer));
            gotEOF = inputBuffer.empty();
            if(gotEOF)
                return eof;
        }
        return static_cast<unsigned char>(inputBuffer[inputPosition]);
    }
    int get()
    {
        int retval = peek();
        if(gotEOF)
        {
            gotEOF = false;
        }
        else
        {
            location = location.getNextLocation(retval, tabSize);
            inputPosition++;
        }
        return retval;
    }
    std::vector<char> getBuffer(std::vector<char> buffer)
    {
        buffer.clear();
        peek();
        if(gotEOF)
        {
            gotEOF = false;
            return buffer;
        }
        else
        {
            if(inputPosition == 0)
            {
                buffer.swap(inputBuffer);
            }
            else
            {
                buffer.assign(inputBuffer.begin() + inputPosition, inputBuffer.end());
            }
            inputPosition = 0;
            for(unsigned char ch : buffer)
            {
                location = location.getNextLocation(ch, tabSize);
            }
            return buffer;
        }
    }
};

class TextInputIStream final : public TextInput
{
private:
    std::istream &is;

protected:
    virtual std::vector<char> fillInputBuffer(std::vector<char> inputBuffer) override
    {
        constexpr std::size_t maxReadSize = 1024;
        inputBuffer.resize(maxReadSize);
        if(!is.fail() && is.eof())
        {
            is.clear();
            inputBuffer.clear();
            return inputBuffer;
        }
        is.peek(); // sets eof flag without setting fail flag
        if(!is.fail() && is.eof())
        {
            is.clear();
            inputBuffer.clear();
            return inputBuffer;
        }
        if(!is.good())
        {
            inputBuffer.clear();
            return inputBuffer;
        }
        inputBuffer[0] = is.get();
        if(is.fail())
        {
            inputBuffer.clear();
            return inputBuffer;
        }
        if(is.eof())
        {
            is.clear();
            inputBuffer.clear();
            return inputBuffer;
        }
        auto readCount = is.readsome(inputBuffer.data() + 1, inputBuffer.size() - 1);
        inputBuffer.resize(readCount + 1);
        return inputBuffer;
    }

public:
    explicit TextInputIStream(std::istream &is, const Location &location = Location()) noexcept
        : TextInput(location), is(is)
    {
    }
};

class TextInputFile final : public TextInput
{
private:
    InputFileDescriptor inputFileDescriptor;
    std::ifstream inputStream;
    TextInputIStream baseTextInput;
    std::istream &setupInputStream(const char *fileName, std::ios::openmode openMode = std::ios::in)
    {
        inputStream.exceptions(std::ios::badbit | std::ios::failbit);
        inputStream.open(fileName, openMode);
        return inputStream;
    }

protected:
    virtual std::vector<char> fillInputBuffer(std::vector<char> inputBuffer) override
    {
        return baseTextInput.getBuffer(std::move(inputBuffer));
    }

public:
    explicit TextInputFile(const char *fileName, std::ios::openmode openMode = std::ios::in)
        : inputFileDescriptor(fileName),
          inputStream(),
          baseTextInput(setupInputStream(fileName, openMode), Location(&inputFileDescriptor))
    {
    	location = baseTextInput.location;
    }
    explicit TextInputFile(const std::string &fileName, std::ios::openmode openMode = std::ios::in)
        : TextInputFile(fileName.c_str(), openMode)
    {
    }
};

class TextInputStdIO final : public TextInput
{
private:
    std::FILE *f;
    bool closeFile;

protected:
    virtual std::vector<char> fillInputBuffer(std::vector<char> inputBuffer) override
    {
        if(std::ferror(f))
        {
            return inputBuffer;
        }
        if(std::feof(f))
        {
            std::clearerr(f);
            return inputBuffer;
        }
        int readValue = std::fgetc(f);
        if(readValue != EOF)
        {
            inputBuffer.push_back(readValue);
            return inputBuffer;
        }
        if(std::ferror(f))
        {
            return inputBuffer;
        }
        if(std::feof(f))
        {
            std::clearerr(f);
            return inputBuffer;
        }
        return inputBuffer;
    }

public:
    TextInputStdIO(std::FILE *f, bool closeFile, const Location &location = Location()) noexcept
        : TextInput(location), f(f), closeFile(closeFile)
    {
    }
    ~TextInputStdIO()
    {
        if(closeFile)
            std::fclose(f);
    }
};
}
}

#endif /* PARSER_TEXT_INPUT_H_ */
