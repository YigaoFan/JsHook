export module Lexer;
// to handle space
import std;
import FuncTrait;

using std::array;
using std::string; // use wstring in the future
using std::pair;
using std::vector;
using std::move;
using std::logic_error;
using std::index_sequence;
using std::make_index_sequence;
using std::make_pair;
using std::cout;
using std::endl;
using std::optional;
using std::forward;
using std::isspace;
using std::string_view;
using std::tuple;

using Token = pair<int, string>;

#define match(s, ...) PatternMatch(s, __VA_ARGS__);
#define pattern_1(pat, pred, handle) make_pair([&]pat{ return pred; }, [=]pat{ return handle; })
#define pattern_2(pat, handle) make_pair([&]pat{ return true; }, [=]pat{ return handle; })
#define EXPAND(X) X
#define GET_4TH_ARG(ARG1, ARG2, ARG3, ARG4, ...) ARG4
#define PATTERN_MACRO_CHOOSER(...) EXPAND(GET_4TH_ARG(__VA_ARGS__, pattern_1, pattern_2,))
#define pattern(...) EXPAND(PATTERN_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__))
/// <typeparam name="Op">lambda</typeparam>
/// <typeparam name="Pred">lambda</typeparam>
template <typename Pred, typename Op, typename... MatchItems>
auto PatternMatch(string str, pair<Pred, Op> matchItem, MatchItems... matchItems)// also support no predicate MatchItem
{
    auto&& pred = matchItem.first;
    auto&& op = matchItem.second;
    constexpr auto predParaCount = lambda_details<Pred>::maximum_argument_count;
    constexpr auto opParacount = lambda_details<Op>::maximum_argument_count;
    static_assert(predParaCount == opParacount, "predicate parameter count should be same as operator");
    constexpr auto count = predParaCount;

    if (str.size() >= count) // count can bigger than str.size()?
    {
        auto invoke = [&str, count]<size_t... Is>(index_sequence<Is...>, auto&& func)
        {
            return func(str[Is]..., str.substr(count - 1));
        };
        auto is = make_index_sequence<count - 1>();
        if (invoke(is, pred)) // or no predicate use overload to implement this
        {
            return invoke(is, op);
        }
    }

    if constexpr (sizeof...(MatchItems) > 0)
    {
        return PatternMatch(move(str), move(matchItems)...); // maybe use forward in the future
    }
    else
    {
        throw logic_error("Pattern matches are non-exhaustive");
    }
}

string DropWhileNotEqualTo(char c, string s)
{
    return s.substr(s.find_first_of(c));
}

string_view TwoCharOps[5] =
{
    "==",
    "!=",
    ">=",
    "<=",
    "->",
};

template <auto N>
bool Contains(string_view (&items)[N], string dest)
{
    for (auto i : items)
    {
        if (i == dest)
        {
            return true;
        }
    }
    return false;
}

template <typename T>
concept CharPred = requires (T t, char c)
{
    { t(c) } -> std::convertible_to<bool>;
};

template <CharPred Pred>
tuple<string, string> Span(Pred predicate, string s)
{
    auto ok = string{ };
    for (size_t i = 0; i < s.size(); i++)
    {
        auto c = s[i];
        if (predicate(c))
        {
            ok.push_back(c);
        }
        else
        {
            return { ok, s.substr(i) };
        }
    }
    return { s, string{} };
}

// add forward type
auto Lex(string str, int line) -> vector<Token>
{
    return match(str,
        pattern((auto c, auto && s), s == "", vector<Token>()),
        pattern((auto c, auto && s), c == '\n', Lex(s, line + 1)),
        pattern((auto c, auto && s), isspace(c), Lex(s, line)),
        pattern((auto c0, auto c1, auto && s), Contains(TwoCharOps, string { c0, c1 }), Lex(s, line)), // TODO change like string { c0, c1 } : Lex(s, line)
        pattern((auto c0, auto c1, auto && s), c0 == '/' && c1 == '/', Lex(DropWhileNotEqualTo('\n', s), line)),
        pattern((auto c, auto && s), Lex(s, line)) // TODO change
    )
}

export
{
    auto Lex(string src, int line) -> vector<Token>;
}