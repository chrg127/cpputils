#pragma once

#include <cmath>
#include <algorithm>
#include <span>
#include "concepts.hpp"

namespace math {

template <Number T>
T ceil_div(T x, T y)
{
    return x/y + (x%y != 0);
}

template <Number T>
constexpr T map(T x, T in_min, T in_max, T out_min, T out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

template <Number T>
T avg(T a, T b)
{
    auto h = std::max(a, b);
    auto l = std::min(a, b);
    return l + (h - l) / 2;
};

template <Number T>
T avg(std::span<T> ns)
{
    auto r = ns[0];
    for (auto i = 1u; i < ns.size(); i++)
        r = avg(r, ns[i]);
    return r;
}

template <Number T>
T percent_of(T x, T max) { return x * 100 / max; }

} // namespace math
