#include <catch2/catch.hpp>
#include <fmt/core.h>
#include "../src/array.hpp"

void test_2dspan()
{
    auto print = [](auto s) {
        for (auto i = 0u; i < s.height(); i++) {
            for (auto j = 0u; j < s.width(); j++)
                fmt::print("{} ", s[i][j]);
            fmt::print("\n");
        }
    };

    auto v = std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    std::span<int> the_span{v};
    auto s = util::Span2D<int>{v, 4, 4};
    auto sub = s.subspan(1, 1, 3, 3);
    auto sub2 = sub.subspan(0, 0, 2, 2);
    print(s);
    print(sub);
    print(sub2);
}

void test_arrays(int x)
{
    void f(const util::HeapArray<int> &a);
    auto a = util::HeapArray<int>(x);
    for (int i = 0; i < x; i++)
        a[i] = i*i;
    for (auto elem : a)
        fmt::print("{}\n", elem);
}

