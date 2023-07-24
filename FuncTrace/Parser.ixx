export module Parser;

import std;
import IParser;

using namespace std;
template <typename T>
using ResultFactory = auto (Text) -> T;

template <typename T>
class WordParser
{
private:
    string mWord;
    ResultFactory<T>* mResultFactory;// why here need to be pointer, but below parameter no need to be. 

public:
    WordParser(string word, ResultFactory<T> resultFactory) : mWord(word), mResultFactory(resultFactory)
    {
    }

    template <ParserInput Input>
    auto Parse(Input input) -> ParserResult<T, Input>
    {
        auto&& word = this->mWord;
        auto t = Text::New();
        // log(`word parse "${word}"`);
        for (auto i = 0; i < word.size(); i++)
        {
            auto const c = input.NextChar();
            if (c.Equal(word[i]))
            {
                t.Append(c);
                continue;
            }
            // log(`failed on ${i}, expect "${word[i]}", actual: "${c}"`);
            return {};
        }

        return ParseSuccessResult
        {
            .Result = this->mResultFactory(t),
            .Remain = input,
        };
    }
};

template <typename T>
concept IIncludes = requires (T t, char c)
{
    { t.includes(c) } -> std::convertible_to<bool>;
};

template <typename T, IIncludes Chars>
class OneOfCharsParser
{
private:
    Chars mChars;
    ResultFactory<T> mResultFactory;

public:
    OneOfCharsParser(IIncludes auto chars, ResultFactory<T> resultFactory)
    {
        this->mChars = chars;
        this->mResultFactory = resultFactory;
    }

    template <ParserInput Input>
    auto Parse(Input input) -> ParserResult<T, Input>
    {
        auto const c = input.NextChar;
        auto&& chars = this->mChars;
        // log('chars', chars, 'c', c.Value);
        if (chars.includes(c.Value))
        {
            return
            {
                .Result = this->mResultFactory(c),
                .Remain = input,
            };
        }
        return {};
    }
};

template <typename T, IIncludes Chars>
class NotParser
{
private:
    Chars mChars;
    ResultFactory<T> mResultFactory;

public:
    NotParser(IIncludes auto chars, ResultFactory<T> resultProcessor)
    {
        this->mChars = chars;
        this->mResultFactory = resultProcessor;
    }

    template <ParserInput Input>
    auto Parse(Input input) -> ParserResult<T, Input>
    {
        auto const c = input.NextChar;
        auto&& chars = this.mChars;
        if (!chars.includes(c.Value))
        {
            return 
            {
                .Result = this.mResultFactory(c),
                .Remain = input,
            };
        }
        else
        {
            return {};
        }
    }
};

export
{
    template <typename T = Text>
    auto MakeWord(string word, ResultFactory<T> resultFactory) -> WordParser<T>
    {
        return WordParser<T>(word, resultFactory);
    }

    template <typename T, IIncludes Chars>
    auto OneOf(IIncludes auto chars, ResultFactory<T> resultProcessor) -> OneOfCharsParser<T, Chars>
    {
        return OneOfCharsParser(chars, resultProcessor);
    }

    template <typename T, IIncludes Chars>
    auto Not(IIncludes auto chars, ResultFactory<T> resultProcessor) -> NotParser<T, Chars>
    {
        return NotParser(chars, resultProcessor);
    }
}