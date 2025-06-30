export module ChartParser:AST;

import std;
import IParser;

using std::variant;
using std::vector;
using std::move;
using std::string;
using std::format;
using std::pair;

// TODO this is not belong to ChartParser AST
class ISyntaxNode
{
    virtual auto ToString() const -> string = 0;
};

class UniversalNode
{
public:
    string const TypeName;
    pair<string, vector<string>> const Rule;
    vector<variant<Text, UniversalNode>> const Children;

    UniversalNode(string typeName, pair<string, vector<string>> rule, vector<variant<Text, UniversalNode>> children)
        : TypeName(move(typeName)), Rule(move(rule)), Children(move(children))
    {

    }

    auto ToString() const -> string
    {
        auto stringify = [this](vector<variant<Text, UniversalNode>> const& children) -> string
            {
                string s = "{";
                for (auto i = 0; auto & x : children)
                {
                    auto& k = this->Rule.second[i];
                    auto v = std::visit([](auto const& x) { return x.ToString(); }, this->Children[i]);
                    s.append(format("{0}: {1},", k, v));
                    ++i;
                }
                s.append("}");
                return s;
            };
        return format("{{ Type: {0}, Children: {1}, }}", this->TypeName, stringify(this->Children));
    }
};

export
{
    ISyntaxNode;
    UniversalNode;
}