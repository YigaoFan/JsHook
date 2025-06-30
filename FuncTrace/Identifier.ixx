export module Identifier;
import Combinator;
import Parser;
import std;

using std::views::transform;
using std::string_view;
using std::views::join;
using std::string;
using std::ranges::to;
using namespace std::literals; // for ""sv

string_view alphabets = "abcdefghijklmnopqrstuvwxyz";
string capAlphabets = alphabets | transform([](auto const& x) { return std::toupper(x); }) | to<string>();
string_view nums = "0123456789";
auto firstChar = join(std::array{ alphabets, string_view(capAlphabets), "_"sv, }) | to<string>();
auto remainChar = join(std::array{ string_view(firstChar), nums, }) | to<string>();

export auto identifier = From(OneOf(firstChar, id))
                .RightWith(From(OneOf(remainChar, id)).ZeroOrMore(combineTexts).Raw(), combine2Text)
                .Raw();