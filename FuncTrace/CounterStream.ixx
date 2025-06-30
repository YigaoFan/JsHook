export module ChartParser:CounterStream;

import IParser;
import std;

using std::move;

template <ParserInput InnerInput>
class CounterStream
{
private:
    InnerInput mStream;
    unsigned mCount;
public:
    CounterStream(InnerInput stream)
        : mStream(move(stream)), mCount(0)
    {
    }

    auto NextChar() -> Char
    {
        this->mCount++;
        return this->mStream.NextChar();
    }

    auto Copy() const -> CounterStream
    {
        auto c = CounterStream(this->mStream.Copy());
        c.mCount = this->mCount;
        return c;
    }

    auto Count() const -> unsigned
    {
        return this->mCount;
    }
};

template <ParserInput InnerInput>
auto NewCounterStream(InnerInput input) -> CounterStream<InnerInput>
{
    return CounterStream(move(input));
}
