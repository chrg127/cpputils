#pragma once

#include <cstddef>
#include <algorithm>
#include <random>
#include <span>
#include <vector>

namespace rng {

inline thread_local auto seed = std::random_device{}();
inline thread_local std::mt19937 rng(seed);

template <std::integral       T = int>   T get()             { return std::uniform_int_distribution <T>(    )(rng); }
template <std::floating_point T = float> T get()             { return std::uniform_real_distribution<T>(    )(rng); }
template <std::integral       T = int>   T between(T x, T y) { return std::uniform_int_distribution <T>(x, y)(rng); }
template <std::floating_point T = float> T between(T x, T y) { return std::uniform_real_distribution<T>(x, y)(rng); }

template <std::integral T = int>
void shuffle_in_place(std::span<T> arr)
{
    for (int i = 0; i < arr.size(); i++)
        std::swap(arr[i], arr[between<T>(0, arr.size() - 1)]);
}

template <std::integral T = int>
std::vector<int> shuffle(std::size_t count)
{
   std::vector<T> result(count);
   std::iota(result.begin(), result.end(), 0);
   shuffle_in_place<T>(result);
   return result;
}

} // namespace random
