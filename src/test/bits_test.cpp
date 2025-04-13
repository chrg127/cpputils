#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>
#include <bits.hpp>

TEST_CASE("setbit()", "[bits]") {
    REQUIRE(bits::setbit(192, 1, 1) == 194);
}

