#include "../src/bits.hpp"
#include <catch2/catch.hpp>
#include <fmt/core.h>

TEST_CASE("setbit()", "[bits]") {
    REQUIRE(bits::setbit(192, 1, 1) == 194);
}

