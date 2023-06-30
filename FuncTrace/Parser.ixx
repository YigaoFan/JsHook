export module Parser;

import std;
import IParser;

using namespace std;
template <typename T>
using ResultFactory = auto (Text) -> T;

template <typename T>
class WordParser : public IParser<T>
{
private:
    string mWord;
    ResultFactory<T>* mResultFactory;// why here need to be pointer, but below parameter no need to be. 

public:

    WordParser(string word, ResultFactory<T> resultFactory) : mWord(word), mResultFactory(resultFactory)
    {
        //this->mWord = word;
        //this->mResultFactory = resultFactory; // without point, compiler thought left is a function
    }

    auto Parse(ParserInput input) -> ParserResult<T> override
    {
        auto&& word = this->mWord;
        auto t = Text::Empty();
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
concept IIncludes = requires (T t)
{
    { t.includes() } -> std::convertible_to<bool>;
};

template <typename T, typename Chars>
requires IIncludes<Chars>
class OneOfCharsParser : public IParser<T>
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

    auto Parse(ParserInput input) -> ParserResult<T> override
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

template <typename T, typename Chars>
requires IIncludes<Chars>
class NotParser : public IParser<T>
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

    auto Parse(ParserInput input) -> ParserResult<T> override
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
    template <typename T>
    auto MakeWord(string word, ResultFactory<T> resultFactory) -> IParserPtr<T>
    {
        // why cannot convert to
        return make_shared<WordParser<T>>(word, resultFactory);
    }

    template <typename T>
    auto OneOf(IIncludes auto chars, ResultFactory<T> resultProcessor) -> IParserPtr<T>
    {
        return make_shared<OneOfCharsParser>(chars, resultProcessor);
    }

    template <typename T>
    auto Not(IIncludes auto chars, ResultFactory<T> resultProcessor) -> IParserPtr<T>
    {
        return make_shared<NotParser>(chars, resultProcessor);
    }
}