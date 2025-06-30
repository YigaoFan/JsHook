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

template <typename Func, typename T>
using ItemsConverterResult = invoke_result_t<Func, vector<T>>;

template <typename T0, typename T1, typename T2>
using ResultConverter = auto (T0, T1) -> T2;

template <typename Func, typename T0, typename T1>
using ResultCombinatorResult = invoke_result_t<Func, T0, T1>;

template <typename T0, typename T1>
using Transformer = auto (T0) -> T1;

template <typename Func, typename T>
using TransformerResult = invoke_result_t<Func, T>;

template <IParser InnerParser, typename Transformer, typename T0 = ResultTypeOfParserResult<InnerParser>, typename T1 = TransformerResult<Transformer, T0>>
class TransformCombinator
{
private:
    InnerParser mParser;
    Transformer mTransformer;
public:
    TransformCombinator(InnerParser parser, Transformer transformer) : mParser(parser), mTransformer(transformer)
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

template <IParser Parser, typename Transformer>
auto Transform(Parser parser, Transformer transformer) -> TransformCombinator<Parser, Transformer>
{
    return TransformCombinator(parser, transformer);
}

template <IParser InnerParser0, IParser InnerParser1, typename ResultCombinator, typename T0 = ResultTypeOfParserResult<InnerParser0>, typename T1 = ResultTypeOfParserResult<InnerParser1>, typename T2 = ResultCombinatorResult<ResultCombinator, T0, T1>>
class CombineCombinator
{
private:
    InnerParser0 mParser0;
    InnerParser1 mParser1;
    ResultCombinator mResultCombinator;
public:
    CombineCombinator(InnerParser0 parser0, InnerParser1 parser1, ResultCombinator resultCombinator)
        : mParser0(parser0), mParser1(parser1), mResultCombinator(resultCombinator)
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
                auto const r2 = this->mResultCombinator(r0->Result, r1->Result);
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

template <IParser Parser0, IParser Parser1, typename ResultCombinator>
auto Combine(Parser0 parser0, Parser1 parser1, ResultCombinator resultCombinator) -> CombineCombinator<Parser0, Parser1, ResultCombinator>
{
    // I remember a place said try best to use brace constructor
    return /*CombineCombinator<T0, T1, T2, Parser0, Parser1>*/{ parser0, parser1, resultCombinator };
}

template <IParser InnerParser, typename T = ResultTypeOfParserResult<InnerParser>>
class OptionCombinator
{
private:
    InnerParser mParser;
public:
    OptionCombinator(InnerParser parser) : mParser(parser)
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

export
{
    template <IParser Parser>
    auto Option(Parser parser) -> OptionCombinator<Parser>
    {
        return OptionCombinator<Parser>(parser); // why here need pass Parser type, cannot deduce from parser variable?
    }
}

template <IParser InnerParser, typename ItemsConverter, typename T0 = ResultTypeOfParserResult<InnerParser>, typename T1 = ItemsConverterResult<ItemsConverter, T0>>
class OneOrMoreCombinator
{
private:
    OptionCombinator<InnerParser, T0> mOption;
    ItemsConverter mResultConverter;
public:
    OneOrMoreCombinator(InnerParser parser, ItemsConverter resultConverter) : mOption(Option(parser)), mResultConverter(resultConverter)
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

template <IParser Parser, typename ItemsConverter>
auto OneOrMore(Parser parser, ItemsConverter resultConverter) -> OneOrMoreCombinator<Parser, ItemsConverter>
{
    return OneOrMoreCombinator(parser, resultConverter);
}

template <IParser InnerParser, typename ItemsConverter, typename T0 = ResultTypeOfParserResult<InnerParser>, typename T1 = ItemsConverterResult<ItemsConverter, T0>>
class ZeroOrMoreCombinator
{
private:
    OptionCombinator<InnerParser, T0> mOption;
    ItemsConverter mResultConverter;
public:
    ZeroOrMoreCombinator(InnerParser parser, ItemsConverter resultConverter) : mOption(Option(parser)), mResultConverter(resultConverter)
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

template <IParser Parser, typename ItemsConverter>
auto ZeroOrMore(Parser parser, ItemsConverter resultConverter) -> ZeroOrMoreCombinator<Parser, ItemsConverter>
{
    return ZeroOrMoreCombinator(parser, resultConverter);
}

template <IParser Parser, typename T = ResultTypeOfParserResult<Parser>>// so good here
class FromCombinator;

export
{
    template <typename Raw>
    auto From(Raw parser) -> FromCombinator<Raw>
    {
        return FromCombinator(parser);
    }
}

template <IParser Parser, typename T>
class FromCombinator
{
private:
    Parser mParser;
public:
    FromCombinator(Parser parser) : mParser(parser)
    {
    }

    template <typename ItemsConverter>
    auto OneOrMore(ItemsConverter resultConverter) -> FromCombinator<OneOrMoreCombinator<Parser, ItemsConverter>>
    {
        return From(::OneOrMore(this->mParser, resultConverter));
    }

    template <typename ItemsConverter>
    auto ZeroOrMore(ItemsConverter resultConverter) -> FromCombinator<ZeroOrMoreCombinator<Parser, ItemsConverter>>
    {
        return From(::ZeroOrMore(this->mParser, resultConverter));
    }
    
    template <IParser Parser1, typename ResultCombinator>
    auto RightWith(Parser1 p1, ResultCombinator resultCombinator) -> FromCombinator<CombineCombinator<Parser, Parser1, ResultCombinator>>
    {
        return From(Combine(this->mParser, p1, resultCombinator));
    }
    // function type has little pattern matching, so can deduce out T1, T2
    template <IParser Parser1, typename ResultCombinator>
    auto LeftWith(Parser1 p1, ResultCombinator resultCombinator) -> FromCombinator<CombineCombinator<Parser1, Parser, ResultCombinator>>
    {
        return From(Combine(p1, this->mParser, resultCombinator));
    }

    template <typename Transformer>
    auto Transform(Transformer transformFunc) -> FromCombinator<TransformCombinator<Parser, Transformer>>
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
    auto id = [](auto o) { return o; };

    //template <typename T>
    //auto id(T o) -> T
    //{
    //    return o;
    //}

    auto nullize = []<typename T>(T const& ts) -> decltype(nullptr) { return nullptr; };
    //template <typename T>
    //auto nullize(vector<T> ts) -> decltype(nullptr)
    //{
    //    return nullptr;
    //}

    auto selectLeft = [](auto t0, auto t1)-> decltype(t0) { return t0; };
    //template <typename T0, typename T1>
    //auto selectLeft(T0 t0, T1 t1) -> T0
    //{
    //    return t0;
    //}

    auto selectRight = [](auto t0, auto t1)-> decltype(t1) { return t1; };
    //template <typename T0, typename T1>
    //auto selectRight(T0 t0, T1 t1) -> T1
    //{
    //    return t1;
    //}
}