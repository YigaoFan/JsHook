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

template <typename T0, typename T1>
class TransformCombinator : public IParser<T1>
{
private:
    IParserPtr<T0> mParser;
    Transformer<T0, T1>* mTransformer;
public:
    TransformCombinator(IParserPtr<T0> parser, Transformer<T0, T1> transformer) : mParser(parser), mTransformer(transformer)
    {
    }

    auto Parse(ParserInput input) -> ParserResult<T1> override
    {
        auto const r = this->mParser->Parse(input);
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

template <typename T0, typename T1>
auto Transform(IParserPtr<T0> parser, Transformer<T0, T1> transformer) -> IParserPtr<T1>
{
    return make_shared<TransformCombinator>(parser, transformer);
}

template <typename T0, typename T1, typename T2>
class CombineCombinator : public IParser<T2>
{
private:
    IParserPtr<T0> mParser0;
    IParserPtr<T1> mParser1;
    ResultConverter<T0, T1, T2>* mResultConverter;

public:
    CombineCombinator(IParserPtr<T0> parser0, IParserPtr<T1> parser1, ResultConverter<T0, T1, T2> resultConverter)
        : mParser0(parser0), mParser1(parser1), mResultConverter(resultConverter)
    {
    }

    auto Parse(ParserInput input) -> ParserResult<T2> override
    {
        auto const r0 = this->mParser0->Parse(input);
        if (r0.has_value())
        {
            auto const r1 = this->mParser1->Parse(r0->Remain);
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

template <typename T0, typename T1, typename T2>
auto Combine(IParserPtr<T0> parser0, IParserPtr<T1> parser1, ResultConverter<T0, T1, T2> resultConverter) -> IParserPtr<T2>
{
    return make_shared<CombineCombinator<T0, T1, T2>>(parser0, parser1, resultConverter);
}

template <typename T>
class OptionCombinator : public IParser<optional<T>>
{
private:
    IParserPtr<T> mParser;
public:
    OptionCombinator(IParserPtr<T> parser) : mParser(parser)
    {
    }

    /// <summary>
    /// return value must have Result and Remain field
    /// </summary>
    auto Parse(ParserInput input) -> ParserResult<optional<T>> override
    {
        auto oldInput = input.Copy();
        auto const r = this->mParser->Parse(input);
        if (not r.has_value())
        {
            return ParseSuccessResult<optional<T>, ParserInput>
            {
                .Result = {},
                .Remain = oldInput,
            };
        }
        return ParseSuccessResult<optional<T>, ParserInput>
        {
            .Result = r->Result,
            .Remain = r->Remain,
        };
    }
};

template <typename T>
auto Option(IParserPtr<T> parser) -> IParserPtr<optional<T>>
{
    return make_shared<OptionCombinator<T>>(parser);
}

template <typename T0, typename T1>
class OneOrMoreCombinator : public IParser<T1>
{
private:
    IParserPtr<optional<T0>> mOption;
    ItemsConverter<T0, T1>* mResultConverter;

public:
    OneOrMoreCombinator(IParserPtr<T0> parser, ItemsConverter<T0, T1> resultConverter) : mOption(Option(parser)), mResultConverter(resultConverter)
    {
    }

    auto Parse(ParserInput input) -> ParserResult<T1> override
    {
        vector<T0> results;
        for (;;)
        {
            auto r = this->mOption->Parse(input);
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

template <typename T, typename T1>
auto OneOrMore(IParserPtr<T> parser, ItemsConverter<T, T1> resultConverter) -> IParserPtr<T1>
{
    return make_shared<OneOrMoreCombinator<T, T1>>(parser, resultConverter);
}

template <typename T0, typename T1>
class ZeroOrMoreCombinator : public IParser<T1>
{
private:
    IParserPtr<optional<T0>> mOption;
    ItemsConverter<T0, T1>* mResultConverter;

public:
    ZeroOrMoreCombinator(IParserPtr<T0> parser, ItemsConverter<T0, T1> resultConverter) : mOption(Option(parser)), mResultConverter(resultConverter)
    {
    }

    auto Parse(ParserInput input) -> ParserResult<T1> override
    {
        auto const r = this->Iter(input, {});
        return ParseSuccessResult
        {
            .Result = this->mResultConverter(r.Result),
            .Remain = r.Remain,
        };
    }

private:
    auto Iter(ParserInput input, vector<T0> ts) -> ParseSuccessResult<vector<T0>, ParserInput>
    {
        auto const r = this->mOption->Parse(input);
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

template <typename T, typename T1>
auto ZeroOrMore(IParserPtr<T> parser, ItemsConverter<T, T1> resultConverter) -> IParserPtr<T1>
{
    return make_shared<ZeroOrMoreCombinator<T, T1>>(parser, resultConverter);
}

template <typename T>
class FromCombinator;

export
{
    template <typename T>
    auto From(IParserPtr<T> parser) -> FromCombinator<T>
    {
        return FromCombinator(parser);
    }
}

template <typename T>
class FromCombinator
{
private:
    IParserPtr<T> mParser;
public:
    FromCombinator(IParserPtr<T> parser) : mParser(parser)
    {
    }

    template <typename T1>
    auto OneOrMore(ItemsConverter<T, T1> resultConverter) -> FromCombinator<T1>
    {
        return From(::OneOrMore(this->mParser, resultConverter));
    }

    template <typename T1>
    auto ZeroOrMore(ItemsConverter<T, T1> resultConverter) -> FromCombinator<T1>
    {
        return From(::ZeroOrMore(this->mParser, resultConverter));
    }
    
    template <typename T1, typename T2>
    auto RightWith(IParserPtr<T1> p1, ResultConverter<T, T1, T2> resultCombinator) -> FromCombinator<T2>
    {
        return From(Combine(this->mParser, p1, resultCombinator));
    }

    template <typename T1, typename T2>
    auto LeftWith(IParserPtr<T1> p1, ResultConverter<T1, T, T2> resultCombinator) -> FromCombinator<T2>
    {
        return From(Combine(p1, this->mParser, resultCombinator));
    }
    template <typename T1>
    auto Transform(Transformer<T, T1> transformFunc) -> FromCombinator<T1>
    {
        return From(::Transform(this->mParser, transformFunc));
    }
    /*auto prefixComment(comment: string) = > from(prefixComment(p, comment)),*/
    auto Raw() -> IParserPtr<T>
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

    template <typename... Ts>
    auto nullize(Ts... ts) -> decltype(nullptr)
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