export module Combinator;

import IParser;
import std;
import Util;

using namespace std;

template <typename T0>
consteval auto IsSameType() -> bool
{
    return true;
}

template <typename T0, typename T1, typename... Ts>
consteval auto IsSameType() -> bool
{
    return is_same_v<T0, T1>&& IsSameType<T1, Ts...>();
}

template <typename T0, typename T1, typename... Ts>
concept AllSame = IsSameType<T0, T1, Ts...>();

template <typename T0, typename T1>
using ItemsConverter = auto (vector<T0>) -> T1;

template <typename T0, typename T1, typename T2>
using ResultConverter = auto (T0, T1) -> T2;

template <typename T0, typename T1>
using Transformer = auto (T0) -> T1;

template <typename T0, typename T1, IParser<T0> InnerParser>
//requires IParser<Parser>// use Input here is not very good. I am wrong, this is for member parser, so it's need.
class TransformCombinator
{
private:
    InnerParser mParser;
    Transformer<T0, T1>* mTransformer;
public:
    TransformCombinator(InnerParser parser, Transformer<T0, T1> transformer) : mParser(parser), mTransformer(transformer)
    {
    }

    template <ParserInput Input>
    auto Parse(Input input) -> ParserResult<T1, Input>
    {
        auto const r = this->mParser.Parse(input);
        if (!r.has_value())
        {
            return r;
        }
        return ParseSuccessResult
        {
            .Result = this->mTransformer(r->Result),
            .Remain = r->Remain,
        };
    }
};

template <typename T0, typename T1, IParser<T0> Parser>
auto Transform(Parser parser, Transformer<T0, T1> transformer) -> TransformCombinator<T0, T1, Parser>
{
    return TransformCombinator(parser, transformer);
}

template <typename T0, typename T1, typename T2, IParser<T0> InnerParser0, IParser<T1> InnerParser1>
class CombineCombinator
{
private:
    InnerParser0 mParser0;
    InnerParser1 mParser1;
    ResultConverter<T0, T1, T2>* mResultConverter;
public:
    CombineCombinator(InnerParser0 parser0, InnerParser1 parser1, ResultConverter<T0, T1, T2> resultConverter)
        : mParser0(parser0), mParser1(parser1), mResultConverter(resultConverter)
    {
    }

    template <ParserInput Input>
    auto Parse(Input input) -> ParserResult<T2, Input>
    {
        auto const r0 = this->mParser0.Parse(input);
        if (r0.has_value())
        {
            auto const r1 = this->mParser1.Parse(r0->Remain);
            if (r1.has_value())
            {
                auto const r2 = this->mResultConverter(r0->Result, r1->Result);
                return ParseSuccessResult
                {
                    .Result = r2,
                    .Remain = r1->Remain,
                };
            }
        }
        return {};
    }
};

template <typename T0, typename T1, typename T2, IParser<T0> Parser0, IParser<T1> Parser1>
auto Combine(Parser0 parser0, Parser1 parser1, ResultConverter<T0, T1, T2> resultConverter) -> CombineCombinator<T0, T1, T2, Parser0, Parser1>
{
    // I remember a place said try best to use brace constructor
    return /*CombineCombinator<T0, T1, T2, Parser0, Parser1>*/{ parser0, parser1, resultConverter };
}

template <typename T, IParser<T> Parser>
class OptionCombinator
{
private:
    Parser mParser;
public:
    OptionCombinator(Parser parser) : mParser(parser)
    {
    }

    /// <summary>
    /// return value must have Result and Remain field
    /// </summary>
    template <ParserInput Input>
    auto Parse(Input input) -> ParserResult<optional<T>, Input>
    {
        auto oldInput = input.Copy();
        auto const r = this->mParser.Parse(input);
        if (not r.has_value())
        {
            return ParseSuccessResult<optional<T>, Input>
            {
                .Result = {},
                .Remain = oldInput,
            };
        }
        return ParseSuccessResult<optional<T>, Input>
        {
            .Result = r->Result,
            .Remain = r->Remain,
        };
    }
};

template <typename T, IParser<T> Parser>
auto Option(Parser parser) -> OptionCombinator<T, Parser>
{
    return OptionCombinator<T, Parser>(parser); // why here need pass Parser type, cannot deduce from parser variable?
}

template <typename T0, typename T1, IParser<T0> InnerParser>
class OneOrMoreCombinator
{
private:
    OptionCombinator<T0, InnerParser> mOption;
    ItemsConverter<T0, T1>* mResultConverter;
public:
    OneOrMoreCombinator(InnerParser parser, ItemsConverter<T0, T1> resultConverter) : mOption(Option<T0>(parser)), mResultConverter(resultConverter)
    {
    }

    template <ParserInput Input>
    auto Parse(Input input) -> ParserResult<T1, Input>
    {
        vector<T0> results;
        for (;;)
        {
            auto r = this->mOption.Parse(input);
            Assert(r.has_value());
            if (r->Result.has_value())
            {
                results.push_back(move(r->Result.value()));
            }
            else
            {
                if (results.empty())
                {
                    return {};
                }
                return ParseSuccessResult
                {
                    .Result = this->mResultConverter(results),
                    .Remain = r->Remain,
                };
            }
        }
    }
};

template <typename T0, typename T1, IParser<T0> Parser>
auto OneOrMore(Parser parser, ItemsConverter<T0, T1> resultConverter) -> OneOrMoreCombinator<T0, T1, Parser>
{
    return OneOrMoreCombinator(parser, resultConverter);
}

template <typename T0, typename T1, IParser<T0> InnerParser>
class ZeroOrMoreCombinator
{
private:
    OptionCombinator<T0, InnerParser> mOption;
    ItemsConverter<T0, T1>* mResultConverter;
public:
    ZeroOrMoreCombinator(InnerParser parser, ItemsConverter<T0, T1> resultConverter) : mOption(Option<T0>(parser)), mResultConverter(resultConverter)
    {
    }

    template <ParserInput Input>
    auto Parse(Input input) -> ParserResult<T1, Input>
    {
        auto const r = this->Iter(input, {});
        return ParseSuccessResult
        {
            .Result = this->mResultConverter(r.Result),
            .Remain = r.Remain,
        };
    }

private:
    template <ParserInput Input>
    auto Iter(Input input, vector<T0> ts) -> ParseSuccessResult<vector<T0>, Input>
    {
        auto const r = this->mOption.Parse(input);
        Assert(r.has_value());
        if (r->Result.has_value())
        {
            ts.push_back(move(r->Result.value()));
            return this->Iter(r->Remain, ts);
        }
        else
        {
            return ParseSuccessResult
            {
                .Result = ts,
                .Remain = r->Remain,
            };
        }
    }
};

template <typename T0, typename T1, IParser<T0> Parser>
auto ZeroOrMore(Parser parser, ItemsConverter<T0, T1> resultConverter) -> ZeroOrMoreCombinator<T0, T1, Parser>
{
    return ZeroOrMoreCombinator(parser, resultConverter);
}

template <typename Raw, typename T = ResultTypeOfParserResult<Raw>, IParser<T> Parser = Raw>// so good here
    //requires IParser<Raw, T>
class FromCombinator;

export
{
    template <typename Raw, typename T = ResultTypeOfParserResult<Raw>> // so good here
        requires IParser<Raw, T>
    auto From(Raw parser) -> FromCombinator<Raw>
    {
        return FromCombinator(parser);
    }
}

template <typename Raw, typename T, IParser<T> Parser>
class FromCombinator
{
private:
    Parser mParser;
public:
    FromCombinator(Raw parser) : mParser(parser)
    {
    }

    template <typename T1>
    auto OneOrMore(ItemsConverter<T, T1> resultConverter) -> FromCombinator<OneOrMoreCombinator<T, T1, Parser>>
    {
        return From(::OneOrMore(this->mParser, resultConverter));
    }

    template <typename T1 = T>
    auto ZeroOrMore(ItemsConverter<T, T1> resultConverter) -> FromCombinator<ZeroOrMoreCombinator<T, T1, Parser>>
    {
        return From(::ZeroOrMore(this->mParser, resultConverter));
    }
    
    template <typename T1, typename T2, typename Raw, IParser<T1> Parser1 = Raw>
    auto RightWith(Raw p1, ResultConverter<T, T1, T2> resultCombinator) -> FromCombinator<CombineCombinator<T, T1, T2, Parser, Parser1>>
    {
        return From(Combine(this->mParser, p1, resultCombinator));
    }
    // function type has little pattern matching, so can deduce out T1, T2
    template <typename T1, typename T2, typename Raw, IParser<T1> Parser1 = Raw>
    auto LeftWith(Raw p1, ResultConverter<T1, T, T2> resultCombinator) -> FromCombinator<CombineCombinator<T1, T, T2, Parser1, Parser>>
    {
        return From(Combine(p1, this->mParser, resultCombinator));
    }

    template <typename T1>
    auto Transform(Transformer<T, T1> transformFunc) -> FromCombinator<TransformCombinator<T, T1, Parser>>
    {
        return From(::Transform(this->mParser, transformFunc));
    }

    auto Raw() -> Parser
    {
        return this->mParser;
    }
};

export
{
    template <typename T>
    auto id(T o) -> T
    {
        return o;
    }

    template <typename T>
    auto nullize(vector<T> ts) -> decltype(nullptr)
    {
        return nullptr;
    }

    template <typename T0, typename T1>
    auto selectLeft(T0 t0, T1 t1) -> T0
    {
        return t0;
    }

    template <typename T0, typename T1>
    auto selectRight(T0 t0, T1 t1) -> T1
    {
        return t1;
    }
}