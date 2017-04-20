// automatically generated from parser.peg
#line 1 "parser.peg"
              
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

#line 21 "parser.cpp"
#include "parser.h"

namespace quick_shell
{
namespace parser
{
Parser::Parser(std::shared_ptr<const char32_t> source, std::size_t sourceSize)
    : resultsPointers(sourceSize, nullptr),
      resultsChunks(),
      eofResults(),
      source(std::move(source)),
      sourceSize(sourceSize)
{
}

Parser::Parser(std::u32string source) : Parser(makeSource(std::move(source)))
{
}

Parser::Parser(const char *source, std::size_t sourceSize) : Parser(makeSource(source, sourceSize))
{
}

Parser::Parser(const char32_t *source, std::size_t sourceSize)
    : Parser(makeSource(std::u32string(source, sourceSize)))
{
}

std::pair<std::shared_ptr<const char32_t>, std::size_t> Parser::makeSource(std::u32string source)
{
    auto sourceSize = source.size();
    auto pSource = std::make_shared<std::u32string>(std::move(source));
    return std::make_pair(std::shared_ptr<const char32_t>(pSource, pSource->data()), sourceSize);
}

std::pair<std::shared_ptr<const char32_t>, std::size_t> Parser::makeSource(const char *source,
                                                                           std::size_t sourceSize)
{
    std::u32string retval;
    retval.reserve(sourceSize);
    std::size_t position = 0;
    const char32_t replacementChar = U'\uFFFD';
    while(position < sourceSize)
    {
        unsigned long byte1 = source[position++];
        if(byte1 < 0x80)
        {
            retval += static_cast<char32_t>(byte1);
            continue;
        }
        if(position >= sourceSize || byte1 < 0xC0 || (source[position] & 0xC0) != 0x80)
        {
            retval += replacementChar;
            continue;
        }
        bool invalid = byte1 < 0xC2 || byte1 > 0xF4;
        unsigned long byte2 = source[position++];
        if(byte1 < 0xE0)
        {
            if(invalid)
                retval += replacementChar;
            else
                retval += static_cast<char32_t>(((byte1 & 0x1F) << 6) | (byte2 & 0x3F));
            continue;
        }
        if(position >= sourceSize || (source[position] & 0xC0) != 0x80)
        {
            retval += replacementChar;
            continue;
        }
        unsigned long byte3 = source[position++];
        if(byte1 < 0xF0)
        {
            if(byte1 == 0xE0 && byte2 < 0xA0)
                invalid = true;
            if(invalid)
                retval += replacementChar;
            else
                retval += static_cast<char32_t>(((byte1 & 0xF) << 12) | ((byte2 & 0x3F) << 6)
                                                | (byte3 & 0x3F));
            continue;
        }
        if(position >= sourceSize || (source[position] & 0xC0) != 0x80)
        {
            retval += replacementChar;
            continue;
        }
        unsigned long byte4 = source[position++];
        if(byte1 == 0xF0 && byte2 < 0x90)
            invalid = true;
        if(byte1 == 0xF4 && byte2 > 0x8F)
            invalid = true;
        if(byte1 > 0xF4)
            invalid = true;
        if(invalid)
            retval += replacementChar;
        else
            retval += static_cast<char32_t>(((byte1 & 0x7) << 18) | ((byte2 & 0x3F) << 12)
                                            | ((byte3 & 0x3F) << 6) | (byte4 & 0x3F));
    }
    return makeSource(std::move(retval));
}

void Parser::parseUnimplemented()
{
    RuleResult result;
    internalParseUnimplemented(0, result, true);
    assert(!result.empty());
    if(result.fail())
        throw ParseError(errorLocation, errorMessage);
}

void Parser::internalParseUnimplemented(std::size_t startLocation__, RuleResult &ruleResultOut__, bool isRequiredForSuccess__)
{
    auto &ruleResult__ = this->getResults(startLocation__).resultUnimplemented;
    if(!ruleResult__.empty() && (ruleResult__.fail() || !isRequiredForSuccess__))
    {
        ruleResultOut__ = ruleResult__;
        return;
    }
    {
        const char *predicateReturnValue__ = nullptr;
        {
#line 32 "parser.peg"
                  (predicateReturnValue__) = "unimplemented";
#line 147 "parser.cpp"
        }
        ruleResult__ = this->makeSuccess(startLocation__);
        if(predicateReturnValue__ != nullptr)
            ruleResult__ = this->makeFail(startLocation__, predicateReturnValue__, isRequiredForSuccess__);
    }
    ruleResultOut__ = ruleResult__;
}

std::string Parser::parseNewLine()
{
    RuleResult result;
    auto retval = internalParseNewLine(0, result, true);
    assert(!result.empty());
    if(result.fail())
        throw ParseError(errorLocation, errorMessage);
    return retval;
}

std::string Parser::internalParseNewLine(std::size_t startLocation__, RuleResult &ruleResultOut__, bool isRequiredForSuccess__)
{
    std::string returnValue__{};
    char32_t ch{};
    auto &ruleResult__ = this->getResults(startLocation__).resultNewLine;
    if(!ruleResult__.empty() && (ruleResult__.fail() || !isRequiredForSuccess__))
    {
        ruleResultOut__ = ruleResult__;
        return returnValue__;
    }
    if(startLocation__ >= this->sourceSize)
    {
        ruleResult__ = this->makeFail(startLocation__, "missing end of line (\'\\r\')", isRequiredForSuccess__);
    }
    else if(this->source.get()[startLocation__] == U'\r')
    {
        ruleResult__ = this->makeSuccess(startLocation__ + 1, startLocation__ + 1);
    }
    else
    {
        ruleResult__ = this->makeFail(startLocation__, startLocation__ + 1, "missing end of line (\'\\r\')", isRequiredForSuccess__);
    }
    if(ruleResult__.success())
    {
        auto savedStartLocation__ = startLocation__;
        startLocation__ = ruleResult__.location;
        if(startLocation__ >= this->sourceSize)
        {
            ruleResult__ = this->makeFail(startLocation__, "missing end of line (\'\\n\')", isRequiredForSuccess__);
        }
        else if(this->source.get()[startLocation__] == U'\n')
        {
            ruleResult__ = this->makeSuccess(startLocation__ + 1, startLocation__ + 1);
        }
        else
        {
            ruleResult__ = this->makeFail(startLocation__, startLocation__ + 1, "missing end of line (\'\\n\')", isRequiredForSuccess__);
        }
        startLocation__ = savedStartLocation__;
    }
    if(ruleResult__.success())
    {
        auto savedStartLocation__ = startLocation__;
        startLocation__ = ruleResult__.location;
        {
#line 34 "parser.peg"
                         (returnValue__) = "\r\n";
#line 213 "parser.cpp"
        }
        ruleResult__ = this->makeSuccess(startLocation__);
        startLocation__ = savedStartLocation__;
    }
    if(ruleResult__.fail())
    {
        Parser::RuleResult lastRuleResult__ = ruleResult__;
        if(startLocation__ >= this->sourceSize)
        {
            ruleResult__ = this->makeFail(startLocation__, "unexpected end of input", isRequiredForSuccess__);
        }
        else
        {
            bool matches = false;
            if(this->source.get()[startLocation__] == U'\n')
            {
                matches = true;
            }
            else if(this->source.get()[startLocation__] == U'\r')
            {
                matches = true;
            }
            if(matches)
            {
                ruleResult__ = this->makeSuccess(startLocation__ + 1, startLocation__ + 1);
                ch = this->source.get()[startLocation__];
            }
            else
            {
                ruleResult__ = this->makeFail(startLocation__, startLocation__ + 1, "missing line ending", isRequiredForSuccess__);
            }
        }
        if(ruleResult__.success())
        {
            auto savedStartLocation__ = startLocation__;
            startLocation__ = ruleResult__.location;
            {
#line 35 "parser.peg"
                            (returnValue__) += ch;
#line 253 "parser.cpp"
            }
            ruleResult__ = this->makeSuccess(startLocation__);
            startLocation__ = savedStartLocation__;
        }
        if(ruleResult__.success())
        {
            if(lastRuleResult__.endLocation >= ruleResult__.endLocation)
            {
                ruleResult__.endLocation = lastRuleResult__.endLocation;
            }
        }
    }
    ruleResultOut__ = ruleResult__;
    return returnValue__;
}

char32_t Parser::parseIfsChar()
{
    RuleResult result;
    auto retval = internalParseIfsChar(0, result, true);
    assert(!result.empty());
    if(result.fail())
        throw ParseError(errorLocation, errorMessage);
    return retval;
}

char32_t Parser::internalParseIfsChar(std::size_t startLocation__, RuleResult &ruleResultOut__, bool isRequiredForSuccess__)
{
    char32_t returnValue__{};
    char32_t value{};
    auto &ruleResult__ = this->getResults(startLocation__).resultIfsChar;
    if(!ruleResult__.empty() && (ruleResult__.fail() || !isRequiredForSuccess__))
    {
        ruleResultOut__ = ruleResult__;
        return returnValue__;
    }
    if(startLocation__ >= this->sourceSize)
    {
        ruleResult__ = this->makeFail(startLocation__, "unexpected end of input", isRequiredForSuccess__);
    }
    else
    {
        bool matches = false;
        if(!matches)
        {
            ruleResult__ = this->makeSuccess(startLocation__ + 1, startLocation__ + 1);
            value = this->source.get()[startLocation__];
        }
        else
        {
            ruleResult__ = this->makeFail(startLocation__, startLocation__ + 1, "[] not allowed here", isRequiredForSuccess__);
        }
    }
    if(ruleResult__.success())
    {
        auto savedStartLocation__ = startLocation__;
        startLocation__ = ruleResult__.location;
        {
            const char *predicateReturnValue__ = nullptr;
            {
#line 38 "parser.peg"
      
        if(ifsValue.find(value) == std::u32string::npos)
            (predicateReturnValue__) = "not an IFS character";
        (returnValue__) = value;
    
#line 320 "parser.cpp"
            }
            ruleResult__ = this->makeSuccess(startLocation__);
            if(predicateReturnValue__ != nullptr)
                ruleResult__ = this->makeFail(startLocation__, predicateReturnValue__, isRequiredForSuccess__);
        }
        startLocation__ = savedStartLocation__;
    }
    ruleResultOut__ = ruleResult__;
    return returnValue__;
}

std::string Parser::parseIfsCharSequence()
{
    RuleResult result;
    auto retval = internalParseIfsCharSequence(0, result, true);
    assert(!result.empty());
    if(result.fail())
        throw ParseError(errorLocation, errorMessage);
    return retval;
}

std::string Parser::internalParseIfsCharSequence(std::size_t startLocation__, RuleResult &ruleResultOut__, bool isRequiredForSuccess__)
{
    std::string returnValue__{};
    char32_t ch{};
    auto &ruleResult__ = this->getResults(startLocation__).resultIfsCharSequence;
    if(!ruleResult__.empty() && (ruleResult__.fail() || !isRequiredForSuccess__))
    {
        ruleResultOut__ = ruleResult__;
        return returnValue__;
    }
    ruleResult__ = Parser::RuleResult();
    ch = this->internalParseIfsChar(startLocation__, ruleResult__, isRequiredForSuccess__);
    assert(!ruleResult__.empty());
    if(ruleResult__.success())
    {
        auto savedStartLocation__ = startLocation__;
        startLocation__ = ruleResult__.location;
        {
#line 44 "parser.peg"
                                      (returnValue__) += ch;
#line 362 "parser.cpp"
        }
        ruleResult__ = this->makeSuccess(startLocation__);
        startLocation__ = savedStartLocation__;
    }
    if(ruleResult__.success())
    {
        auto savedStartLocation__ = startLocation__;
        auto &savedRuleResult__ = ruleResult__;
        while(true)
        {
            Parser::RuleResult ruleResult__;
            startLocation__ = savedRuleResult__.location;
            ruleResult__ = Parser::RuleResult();
            ch = this->internalParseIfsChar(startLocation__, ruleResult__, isRequiredForSuccess__);
            assert(!ruleResult__.empty());
            if(ruleResult__.success())
            {
                auto savedStartLocation__ = startLocation__;
                startLocation__ = ruleResult__.location;
                {
#line 44 "parser.peg"
                                      (returnValue__) += ch;
#line 385 "parser.cpp"
                }
                ruleResult__ = this->makeSuccess(startLocation__);
                startLocation__ = savedStartLocation__;
            }
            if(ruleResult__.fail() || ruleResult__.location == startLocation__)
            {
                savedRuleResult__ = this->makeSuccess(savedRuleResult__.location, ruleResult__.endLocation);
                startLocation__ = savedStartLocation__;
                break;
            }
            savedRuleResult__ = this->makeSuccess(ruleResult__.location, ruleResult__.endLocation);
        }
    }
    ruleResultOut__ = ruleResult__;
    return returnValue__;
}

void Parser::parseVariableAssignment()
{
    RuleResult result;
    internalParseVariableAssignment(0, result, true);
    assert(!result.empty());
    if(result.fail())
        throw ParseError(errorLocation, errorMessage);
}

void Parser::internalParseVariableAssignment(std::size_t startLocation__, RuleResult &ruleResultOut__, bool isRequiredForSuccess__)
{
    Parser::RuleResult ruleResult__;
    ruleResult__ = Parser::RuleResult();
    this->internalParseUnimplemented(startLocation__, ruleResult__, isRequiredForSuccess__);
    assert(!ruleResult__.empty());
    ruleResultOut__ = ruleResult__;
}

void Parser::parseWord()
{
    RuleResult result;
    internalParseWord(0, result, true);
    assert(!result.empty());
    if(result.fail())
        throw ParseError(errorLocation, errorMessage);
}

void Parser::internalParseWord(std::size_t startLocation__, RuleResult &ruleResultOut__, bool isRequiredForSuccess__)
{
    Parser::RuleResult ruleResult__;
    ruleResult__ = Parser::RuleResult();
    this->internalParseUnimplemented(startLocation__, ruleResult__, isRequiredForSuccess__);
    assert(!ruleResult__.empty());
    ruleResultOut__ = ruleResult__;
}

void Parser::parseControlOperator()
{
    RuleResult result;
    internalParseControlOperator(0, result, true);
    assert(!result.empty());
    if(result.fail())
        throw ParseError(errorLocation, errorMessage);
}

void Parser::internalParseControlOperator(std::size_t startLocation__, RuleResult &ruleResultOut__, bool isRequiredForSuccess__)
{
    Parser::RuleResult ruleResult__;
    ruleResult__ = Parser::RuleResult();
    this->internalParseUnimplemented(startLocation__, ruleResult__, isRequiredForSuccess__);
    assert(!ruleResult__.empty());
    ruleResultOut__ = ruleResult__;
}

void Parser::parseSimpleCommand()
{
    RuleResult result;
    internalParseSimpleCommand(0, result, true);
    assert(!result.empty());
    if(result.fail())
        throw ParseError(errorLocation, errorMessage);
}

void Parser::internalParseSimpleCommand(std::size_t startLocation__, RuleResult &ruleResultOut__, bool isRequiredForSuccess__)
{
    auto &ruleResult__ = this->getResults(startLocation__).resultSimpleCommand;
    if(!ruleResult__.empty() && (ruleResult__.fail() || !isRequiredForSuccess__))
    {
        ruleResultOut__ = ruleResult__;
        return;
    }
    ruleResult__ = this->makeSuccess(startLocation__);
    {
        auto savedStartLocation__ = startLocation__;
        auto &savedRuleResult__ = ruleResult__;
        while(true)
        {
            Parser::RuleResult ruleResult__;
            startLocation__ = savedRuleResult__.location;
            ruleResult__ = Parser::RuleResult();
            this->internalParseVariableAssignment(startLocation__, ruleResult__, isRequiredForSuccess__);
            assert(!ruleResult__.empty());
            if(ruleResult__.success())
            {
                auto savedStartLocation__ = startLocation__;
                startLocation__ = ruleResult__.location;
                ruleResult__ = Parser::RuleResult();
                this->internalParseIfsCharSequence(startLocation__, ruleResult__, isRequiredForSuccess__);
                assert(!ruleResult__.empty());
                startLocation__ = savedStartLocation__;
            }
            if(ruleResult__.fail() || ruleResult__.location == startLocation__)
            {
                savedRuleResult__ = this->makeSuccess(savedRuleResult__.location, ruleResult__.endLocation);
                startLocation__ = savedStartLocation__;
                break;
            }
            savedRuleResult__ = this->makeSuccess(ruleResult__.location, ruleResult__.endLocation);
        }
    }
    if(ruleResult__.success())
    {
        auto savedStartLocation__ = startLocation__;
        startLocation__ = ruleResult__.location;
        ruleResult__ = Parser::RuleResult();
        this->internalParseWord(startLocation__, ruleResult__, isRequiredForSuccess__);
        assert(!ruleResult__.empty());
        startLocation__ = savedStartLocation__;
    }
    if(ruleResult__.success())
    {
        auto savedStartLocation__ = startLocation__;
        startLocation__ = ruleResult__.location;
        ruleResult__ = this->makeSuccess(startLocation__);
        {
            auto savedStartLocation__ = startLocation__;
            auto &savedRuleResult__ = ruleResult__;
            while(true)
            {
                Parser::RuleResult ruleResult__;
                startLocation__ = savedRuleResult__.location;
                ruleResult__ = Parser::RuleResult();
                this->internalParseIfsCharSequence(startLocation__, ruleResult__, isRequiredForSuccess__);
                assert(!ruleResult__.empty());
                if(ruleResult__.success())
                {
                    auto savedStartLocation__ = startLocation__;
                    startLocation__ = ruleResult__.location;
                    ruleResult__ = Parser::RuleResult();
                    this->internalParseWord(startLocation__, ruleResult__, isRequiredForSuccess__);
                    assert(!ruleResult__.empty());
                    startLocation__ = savedStartLocation__;
                }
                if(ruleResult__.fail() || ruleResult__.location == startLocation__)
                {
                    savedRuleResult__ = this->makeSuccess(savedRuleResult__.location, ruleResult__.endLocation);
                    startLocation__ = savedStartLocation__;
                    break;
                }
                savedRuleResult__ = this->makeSuccess(ruleResult__.location, ruleResult__.endLocation);
            }
        }
        startLocation__ = savedStartLocation__;
    }
    if(ruleResult__.success())
    {
        auto savedStartLocation__ = startLocation__;
        startLocation__ = ruleResult__.location;
        ruleResult__ = Parser::RuleResult();
        this->internalParseIfsCharSequence(startLocation__, ruleResult__, isRequiredForSuccess__);
        assert(!ruleResult__.empty());
        if(ruleResult__.fail())
            ruleResult__ = this->makeSuccess(startLocation__);
        startLocation__ = savedStartLocation__;
    }
    if(ruleResult__.success())
    {
        auto savedStartLocation__ = startLocation__;
        startLocation__ = ruleResult__.location;
        ruleResult__ = Parser::RuleResult();
        this->internalParseControlOperator(startLocation__, ruleResult__, isRequiredForSuccess__);
        assert(!ruleResult__.empty());
        startLocation__ = savedStartLocation__;
    }
    ruleResultOut__ = ruleResult__;
}
}
}
