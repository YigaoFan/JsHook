import std;
import Combinator;
import Parser;
import IParser;

using namespace std;

template <typename T0, typename T1, typename... Ts>
struct Get2Types
{
    using First = T0;
    using Second = T1;
};

template <typename T0>
consteval auto IsSameType() -> bool
{
    return true;
}

template <typename T0, typename T1, typename... Ts>
consteval auto IsSameType() -> bool
{
    return is_same_v<T0, T1> && IsSameType<T1, Ts...>();
}



template <typename T0, typename T1, typename... Ts>
concept IsAllSame = IsSameType<T0, T1, Ts...>();

template <typename... Ts>
requires IsAllSame<Ts...>
int add(Ts... ts)
{
    return (0 + ... + ts);
}

void apply(int (*func)())
{
    func();
}

auto Cons0()
{
    struct Type { int Num = 1; char c; } a;
    return a;
}

auto Cons1()
{
    struct Type { int Num = 1; } a;
    return a;
}

//template <typename T>
//auto id(T o) -> T
//{
//    return o;
//}
//
//template <typename T>
//using Callback = auto (int)->T;
//
//template <typename T>
//auto Transform(Callback<T> callback) -> T
//{
//    int i = 1;
//    return callback(i);
//}

struct A
{
    int Num;
    string Str;
};
int main()
{
    auto o = A{ 1, "Hello world"};
    cout << o.Num;
    //cout << Transform(id<int>); // only id is not a complete type I think, so need to instaniate it
    From(MakeWord("function", id<Text>))
        .RightWith(MakeWord(" ", id<Text>), selectLeft<Text, Text>)
        .LeftWith(MakeWord("Haha", id<Text>), selectLeft<Text, Text>)
        //.OneOrMore(id<vector<Text>>)
        .ZeroOrMore(id<vector<Text>>)
        .Raw();
}