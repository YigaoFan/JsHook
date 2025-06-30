export module Parser;

import std;
import IParser;

using namespace std;
template <typename T>
using ResultFactory = auto (Text) -> T;

template <typename Func>
using ResultFactoryResult = invoke_result_t<Func, Text>;

template <typename ResultFactory, typename T = ResultFactoryResult<ResultFactory>>
class WordParser
{
private:
    string mWord;
    ResultFactory mResultFactory;
    // why here need to be pointer, but below parameter no need to be. because function type cannot be property type directly? and function and function pointer are the same.

public:
    WordParser(string word, ResultFactory resultFactory) : mWord(word), mResultFactory(resultFactory)
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
    { t.contains(c) } -> std::convertible_to<bool>;
};

template <typename ResultFactory, IIncludes Chars, typename T = ResultFactoryResult<ResultFactory>>
class OneOfCharsParser
{
private:
    Chars mChars;
    ResultFactory mResultFactory;

public:
    OneOfCharsParser(Chars chars, ResultFactory resultFactory)
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
        if (chars.contains(c.Value))
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

template <typename ResultFactory, IIncludes Chars, typename T = ResultFactoryResult<ResultFactory>>
class NotParser
{
private:
    Chars mChars;
    ResultFactory mResultFactory;

public:
    NotParser(Chars chars, ResultFactory resultProcessor)
    {
        this->mChars = chars;
        this->mResultFactory = resultProcessor;
    }

    template <ParserInput Input>
    auto Parse(Input input) -> ParserResult<T, Input>
    {
        auto const c = input.NextChar;
        auto&& chars = this.mChars;
        if (!chars.contains(c.Value))
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
    template <typename ResultFactory>
    auto MakeWord(string word, ResultFactory resultFactory) -> WordParser<ResultFactory>
    {
        return WordParser(word, resultFactory);
    }

    template <typename ResultFactory, IIncludes Chars>
    auto OneOf(Chars chars, ResultFactory resultProcessor) -> OneOfCharsParser<ResultFactory, Chars>
    {
        return OneOfCharsParser(chars, resultProcessor);
    }

    template <typename ResultFactory, IIncludes Chars>
    auto Not(Chars chars, ResultFactory resultProcessor) -> NotParser<ResultFactory, Chars>
    {
        return NotParser(chars, resultProcessor);
    }
}