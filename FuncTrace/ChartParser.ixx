export module ChartParser;

import IParser;
import :AST;

class ChartParser
{
private:

public:
    template <ParserInput Input>
    auto Parse(Input input) -> ParserResult<UniversalNode, Input>
    {

    }
};

export
{
    ChartParser;
}