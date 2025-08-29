#include <inplace_vector.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("InPlaceVector test", "[inplace_vector]")
{
    auto v = InPlaceVector<int, 2> { 1, 2 };
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);
}

