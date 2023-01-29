#include <catch2/catch.hpp>
#include <fmt/core.h>
#include "../src/random.hpp"

void test_random()
{
    fmt::print("sizeof dist = {}\n", sizeof(std::uniform_int_distribution<int>));
    fmt::print("seed = ");
    for (auto x : rng::seed)
        fmt::print("{} ", x);
    fmt::print("\n");
    for (int i = 0; i < 10; i++)  { fmt::print("{} ", rng::get<int>());                 } fmt::print("\n");
    for (int i = 0; i < 10; i++)  { fmt::print("{} ", rng::get<float>());               } fmt::print("\n");
    for (int i = 0; i < 25; i++)  { fmt::print("{} ", rng::between(10, 20));            } fmt::print("\n");
    for (int i = 0; i < 10; i++)  { fmt::print("{} ", rng::between<float>(10, 20));     } fmt::print("\n");
    for (int  i = 0; i < 10; i++) { fmt::print("{} ", rng::between<float>(0.0f, 1.0f)); } fmt::print("\n");
    fmt::print("hello, {}\n", rng::pick({ "officer", "engineer", "office worker", "judge" }));
}
