export module Whitespace;
import Combinator;
import std;
import Parser;

using std::string;

string spaces = " \t\n";
export auto whitespace = From(OneOf(spaces, nullize)).OneOrMore(nullize).Raw();