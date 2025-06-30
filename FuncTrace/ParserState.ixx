export module ChartParser:ParserState;

import :GrammarMap;
import IParser;
import std;
import :AST;
import InputStream;
import :CounterStream;

using std::vector;
using std::variant;
using std::move;
using std::string;
using std::runtime_error;
using std::format;

template <typename T>
using Ptr = std::shared_ptr<T>; // maybe change to unique_ptr in future

#define nameof(x) #x

enum class ParserWorkState
{
    Pending,
    Succeed,
    Fail,
};

template <ParserInput Input>
class NonTerminatedParserState
{
public:
    unsigned const From;
    NonTerminatedRule Rule;
    unsigned NowPoint;
private:
    using Node = variant<ParserResult<Text, Input>, ParserResult<decltype(nullptr), Input>, ParserResult<Ptr<ISyntaxNode>, Input>>;
    vector<Node> mNodes;
    Input mInitialInput;

public:
    NonTerminatedParserState(unsigned from, NonTerminatedRule rule, unsigned nowPoint, Input startInput, vector<Node> nodes = {})
        : From(from), Rule(move(rule)), NowPoint(nowPoint), mInitialInput(move(startInput)), mNodes(move(nodes))
    {
    }

    auto operator==(NonTerminatedParserState const& that) const -> bool
    {

    }
};

template <ParserInput Input>
auto NewNonTerminParserState(unsigned from, NonTerminatedRule rule, Input startInput) -> NonTerminatedParserState<Input>
{
    return NonTerminatedParserState(from, move(rule), 0, move(startInput));
}

template <typename T, ParserInput Input>
class TerminatedParserState
{
public:
    unsigned const From;
    Symbol const LeftSymbol;
private:
    ParserResult<T, Input> mResult;
    unsigned mRemainShiftCharCount;
    unsigned const mShiftCharCount;

public:
    TerminatedParserState(unsigned from, Symbol symbol, ParserResult<T, Input> result, unsigned shiftCharCount)
        : From(from), LeftSymbol(move(symbol)), mResult(move(result)), mRemainShiftCharCount(shiftCharCount), mShiftCharCount(shiftCharCount)
    {
    }

    auto operator==(TerminatedParserState const& that) const -> bool
    {
        return this->Rule == that.Rule && this->From == that.From;
    }

    auto Result() const -> decltype(this->mResult)
    {
        if (this->mResult.has_value())
        {
            return this->mResult;
        }
        throw runtime_error("no result in " nameof(TerminatedParserState));
    }

    auto Move() -> ParserWorkState
    {
        if (not this->mResult.has_value())
        {
            return ParserWorkState::Fail;
        }

        --this->mRemainShiftCharCount;
        if (this->mRemainShiftCharCount == 0)
        {
            return ParserWorkState::Succeed;
        }
        return ParserWorkState::Pending;
    }

    auto Completed() const -> bool
    {
        if (not this->mResult.has_value())
        {
            return true;
        }
        return not (this->mRemainShiftCharCount > 0);
    }

    auto State() const -> ParserWorkState
    {
        if (not this->mResult.has_value())
        {
            return ParserWorkState::Fail;
        }
        if (this->mRemainShiftCharCount > 0)
        {
            return ParserWorkState::Pending;
        }
        return ParserWorkState::Succeed;
    }

    auto ToString() const -> string
    {
        return format("{0} from {1} need shift {2} char", this->Symbol, this->From, this->mShiftCharCount);
    }
};

template <typename T, ParserInput Input, IParser Parser>
auto NewTerminParserState(unsigned from, TerminatedRule<Parser> const& rule, Input input) -> TerminatedParserState<T, Input>
{
    auto cs = CounterStream(move(input));
    auto& parser = rule.Parser;
    auto r = parser.Parse(cs);
    unsigned shiftCharCount = 0;
    if (r.has_value())
    {
        shiftCharCount = r.value().Remain.Count();
    }
    return TerminatedParserState(from, rule.Symbol, r, shiftCharCount);
}
