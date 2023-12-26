#include "../array.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Simple test for Span2D", "[array]")
{
    auto v = std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    std::span<int> the_span{v};
    auto s1 = util::Span2D<int>{v, 4, 4};
    auto s2 = s1.subspan(1, 1, 3, 3);
    auto s3 = s2.subspan(0, 0, 2, 2);

    REQUIRE(s1[0][0] == 0);
    REQUIRE(s1[0][1] == 1);
    REQUIRE(s1[0][2] == 2);
    REQUIRE(s1[0][3] == 3);
    REQUIRE(s1[1][0] == 4);
    REQUIRE(s1[1][1] == 5);
    REQUIRE(s1[1][2] == 6);
    REQUIRE(s1[1][3] == 7);
    REQUIRE(s1[2][0] == 8);
    REQUIRE(s1[2][1] == 9);
    REQUIRE(s1[2][2] == 10);
    REQUIRE(s1[2][3] == 11);
    REQUIRE(s1[3][0] == 12);
    REQUIRE(s1[3][1] == 13);
    REQUIRE(s1[3][2] == 14);
    REQUIRE(s1[3][3] == 15);

    REQUIRE(s2[0][0] == 5);
    REQUIRE(s2[0][1] == 6);
    REQUIRE(s2[0][2] == 7);
    REQUIRE(s2[1][0] == 9);
    REQUIRE(s2[1][1] == 10);
    REQUIRE(s2[1][2] == 11);
    REQUIRE(s2[2][0] == 13);
    REQUIRE(s2[2][1] == 14);
    REQUIRE(s2[2][2] == 15);

    REQUIRE(s3[0][0] == 5);
    REQUIRE(s3[0][1] == 6);
    REQUIRE(s3[1][0] == 9);
    REQUIRE(s3[1][1] == 10);
}
