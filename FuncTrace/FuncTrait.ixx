export module FuncTrait;

import std;

// below code from: https://stackoverflow.com/a/54394184
struct any_argument
{
    template <typename T>
    operator T && () const;

    bool operator==(auto&& other) const; // why this make compiler happy?
};

template <typename Lambda, typename Is, typename = void>
struct can_accept_impl : std::false_type
{
};

template <typename Lambda, std::size_t ...Is>
struct can_accept_impl<Lambda, std::index_sequence<Is...>, decltype(std::declval<Lambda>()(((void)Is, any_argument{})...), void())> : std::true_type
{
};

template <typename Lambda, std::size_t N>
struct can_accept : can_accept_impl<Lambda, std::make_index_sequence<N>>
{
};

template <typename Lambda, std::size_t N, size_t Max, typename = void>
struct lambda_details_maximum
{
    static constexpr size_t maximum_argument_count = N - 1;
    static constexpr bool is_variadic = false;
};

template <typename Lambda, std::size_t N, size_t Max>
struct lambda_details_maximum<Lambda, N, Max, std::enable_if_t<can_accept<Lambda, N>::value && (N <= Max)>> : lambda_details_maximum<Lambda, N + 1, Max>
{
};

template <typename Lambda, std::size_t N, size_t Max>
struct lambda_details_maximum<Lambda, N, Max, std::enable_if_t<can_accept<Lambda, N>::value && (N > Max)>>
{
    static constexpr bool is_variadic = true;
};

template <typename Lambda, std::size_t N, size_t Max, typename = void>
struct lambda_details_minimum : lambda_details_minimum<Lambda, N + 1, Max>
{
    static_assert(N <= Max, "Argument limit reached");
};

template <typename Lambda, std::size_t N, size_t Max>
struct lambda_details_minimum<Lambda, N, Max, std::enable_if_t<can_accept<Lambda, N>::value>> : lambda_details_maximum<Lambda, N, Max>
{
    static constexpr size_t minimum_argument_count = N;
};

template <typename Lambda, size_t Max>
struct lambda_details : lambda_details_minimum<Lambda, 0, Max>
{
};

export
{
    template <typename Lambda, size_t Max = 50>
    struct lambda_details;
}