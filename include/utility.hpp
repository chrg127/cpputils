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

template <size_t N>
auto template_for_i(auto &&fn)
{
    [&]<size_t... Is>(std::index_sequence<Is...>) {
        return (fn.template operator()<Is>(), ...);
    }(std::make_index_sequence<N>());
}

template <typename T>
std::span<const u8> as_byte_span(const T &obj)
{
    return std::span<const u8>(reinterpret_cast<const u8 *>(&obj), sizeof(T));
}

template <typename T>
std::span<u8> as_byte_span(T &obj)
{
    return std::span<u8>(reinterpret_cast<u8 *>(&obj), sizeof(T));
}

} // namespace util
