#include <catch2/catch.hpp>
#include <fmt/core.h>
#include "../src/bits.hpp"

TEST_CASE("setbit()", "[bits]") {
    REQUIRE(bits::setbit(192, 1, 1) == 194);
}

