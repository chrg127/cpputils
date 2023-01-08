#pragma once

#include <cstring>
#include <functional>
#include <optional>

namespace util {

template <typename M>
std::optional<typename M::mapped_type> map_lookup(const M &m, const typename M::key_type &key)
{
    auto it = m.find(key);
    if (it == m.end())
        return std::nullopt;
    return it->second;
}

template <typename T, typename R, typename... Args>
std::function<R(Args...)> member_fn(T *obj, R (T::*fn)(Args...))
{
    return [=](Args&&... args) -> R { return (obj->*fn)(args...); };
}

template <typename T>
T ceil_div(T x, T y)
{
    return x/y + (x%y != 0);
}

} // namespace util
