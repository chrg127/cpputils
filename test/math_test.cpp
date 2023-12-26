#include "../src/math.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Simple test for ceiling division", "[math]")
{
    REQUIRE(math::ceil_div(1, 3) == 1);
}

