#pragma once

#include <cmath>
#include <algorithm>
#include <span>
#include <cstdint>
#include <type_traits>

namespace math {

template <typename T> requires std::is_arithmetic<T>
T ceil_div(T x, T y)
{
    return x/y + (x%y != 0);
}

template <typename T> requires std::is_arithmetic<T>
constexpr T map(T x, T in_min, T in_max, T out_min, T out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

template <typename T> requires std::is_arithmetic<T>
T avg(T a, T b)
{
    auto h = std::max(a, b);
    auto l = std::min(a, b);
    return l + (h - l) / 2;
};

template <typename T> requires std::is_arithmetic<T>
T avg(std::span<T> ns)
{
    auto r = ns[0];
    for (auto i = 1u; i < ns.size(); i++)
        r = avg(r, ns[i]);
    return r;
}

template <typename T> requires std::is_arithmetic<T>
T percent_of(T x, T max) { return x * 100 / max; }

template <std::integral T>
T log2i(T x)
{
    return std::bit_width(static_cast<uint64_t>(x)) - 1;
}

// Returns the smallest power of 2 that is >= the given value
template <typename T>
T pow2gt(T x)
{
    T y = 1;
    while (y < x) {
        y *= 2;
    }
    return y;
}

// Returns the largest power of 2 that is <= the given value.
template <typename T>
constexpr inline T pow2lt(T n)
    requires std::is_unsigned_v<T>
{
    // Set all bits less significant than most significant bit that's set.
    // e.g. 0b00100111  ->  0b00111111
    n |= (n >> 1);
    n |= (n >> 2);
    n |= (n >> 4);
    if constexpr(std::numeric_limits<T>::digits == 16) n |= (n >>  8);
    if constexpr(std::numeric_limits<T>::digits == 32) n |= (n >> 16);
    if constexpr(std::numeric_limits<T>::digits == 64) n |= (n >> 32);
    // Clear all set bits besides the highest one.
    return n - (n >> 1);
}

} // namespace math
