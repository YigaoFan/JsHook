export module IParser;

import std;
import Util;

using std::optional;
using std::string;
using std::shared_ptr;
using std::vector;
using std::string;
using std::move;

export class Char
{
private:
    char const mChar;
public:
    Char(char ch) : mChar(ch)
    {
    }

    auto Equal(char c) const -> bool
    {
        return c == this->mChar;
    }
};

class Text 
{
private:
    vector<Char> mChars;
public:
    static auto New(vector<Char> chars = {}) -> Text
    {
        return Text{ chars, };
    }

    Text(vector<Char> chars = {}) : mChars(chars)
    {
    }

    auto Append(Char ch) -> void
    {
        this->mChars.push_back(move(ch));
    }

    /// <param name="start"></param>
    /// <param name="end"></param>
    /// <returns></returns>
    auto SubText(int start, int end = -1) const -> Text
    {
        Assert(start >= -1 && end >= -1, "start and end should bigger than -1");
        if (start == -1)
        {
            return Text{};
        }
        auto b = this->mChars.cbegin() + start;
        auto e = end == -1 ? this->mChars.cend() : this->mChars.cbegin() + end;
        auto chars = vector(b, e);
        return Text{ move(chars), };
    }

    auto Empty() const -> bool
    {
        return this->mChars.empty();
    }
};


export using Text = Text;

template <typename T>
concept IInputStream = requires (T t)
{
    { t.NextChar() } -> std::convertible_to<Char>;
    { t.Copy() } -> std::convertible_to<T>;
};

struct FakeInputStream
{
    auto NextChar() -> Char
    {
        throw;
    }
    auto Copy() -> FakeInputStream
    {
        throw;
    }
};

template <typename Raw>
struct GetResultTypeOfParserResult
{
private:
    using OptionalValue = std::invoke_result<decltype(&Raw::template Parse<FakeInputStream>), Raw, FakeInputStream>::type::value_type;
public:
    using Result = decltype(OptionalValue::Result);
};

export
{
    template <typename T, IInputStream Input>
    struct ParseSuccessResult
    {
        T Result;
        Input Remain;
    };
    template <typename T>
    concept ParserInput = IInputStream<T>; //use concept to limit base IInputStream
    template <typename T, ParserInput Input>
    using ParserResult = optional<ParseSuccessResult<T, Input>>;
    //template <typename T>
    /*struct IParser
    {
        template <ParserInput Input>
        virtual auto Parse(Input input) -> ParserResult<T> = 0;
    };*/
    template <typename T>
    using ResultTypeOfParserResult = typename GetResultTypeOfParserResult<T>::Result;
    // no dynamic need, no need use abstract class
    template <typename Parser>
    concept IParser = requires (Parser parser, FakeInputStream input)
    {
        // actually, the concept don't need or care the T in ParserResult<T... in result of Parse method, just get type from itself by using ResultTypeOfParserResult
        { parser.Parse(input) } -> std::convertible_to<ParserResult<ResultTypeOfParserResult<Parser>, FakeInputStream>>;
    };
}