export module ChartParser:GrammarMap;

import std;
import IParser;

using std::pair;
using std::vector;
using std::string;
using std::shared_ptr;
using std::variant;

// Rule will be shared in many place, so let it be pointer, or make it as a type for usage convenient in the future
using NonTerminatedRule = shared_ptr<pair<string, vector<string>>>;
//using TerminatedRule = shared_ptr<pair<string, string>>; // TODO fix
using Symbol = string;

template <IParser Parser>
class TerminatedRule
{
public:
    Symbol const Symbol;
    Parser Parser;

    auto operator==(TerminatedRule const& that) const
    {
        // Assume in a grammar a symbol is identical
        return this->Symbol == that.Symbol;
    }
};