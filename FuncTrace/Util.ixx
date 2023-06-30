export module Util;

import std;

using std::exception;

export
{
    void Assert(bool condition, char const* message = "")
    {
        if (!condition)
        {
            throw exception(message);
        }
    }
}