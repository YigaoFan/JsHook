export module IParser;

import std;

using std::optional;
using std::string;
using std::shared_ptr;

class Text 
{
public:
    static auto Empty() -> Text
    {
        return Text{};
    }

    auto Equal(string s) const -> bool
    {
        return true; // TODO
    }

    auto Equal(char s) const -> bool
    {
        return true; // TODO
    }

    auto Append(Text t) -> void
    {
        // TODO
    }
};


export using Text = Text;

//struct IInputStream
//{
//    virtual auto NextChar() -> Text// = 0;
//    {
//        throw;
//    }
//    virtual auto Copy() -> IInputStream// = 0;
//    {
//        throw;
//    }
//};

template <typename T>
concept IInputStream = requires (T t)
{
    { t.NextChar() } -> std::convertible_to<Text>;
    { t.Copy() } -> std::convertible_to<T>;
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
    template <typename T>
    /*struct IParser
    {
        template <ParserInput Input>
        virtual auto Parse(Input input) -> ParserResult<T> = 0;
    };*/

    template <typename Parser, typename T, typename Input>
    concept IParser = requires (Parser parser)
    {
        { parser.Parse() } -> std::convertible_to<ParserResult<T, Input>>;
    };
    template <typename T>
    using IParserPtr = shared_ptr<IParser<T>>;
}