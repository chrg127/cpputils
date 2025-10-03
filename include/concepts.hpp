#pragma once

#include <concepts>

namespace util {

template <typename T> concept arithmetic = std::integral<T> || std::floating_point<T>;

template <typename T> concept container = requires(T t) {
    t.data();
    t.size();
};

} // util
