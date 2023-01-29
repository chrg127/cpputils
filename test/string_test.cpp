#include <catch2/catch.hpp>
#include <fmt/core.h>
#include "../src/string.hpp"

TEST_CASE("Split a string", "[string]") {
    std::string test = "1,2,3";
    auto r = string::split(test);
    REQUIRE(r.size() == 3);
    REQUIRE(r[0] == "1");
    REQUIRE(r[1] == "2");
    REQUIRE(r[2] == "3");
}

TEST_CASE("Split a string with space delimiter and using string_view", "[string]") {
    std::string_view str = "hello world hi";
    auto r = string::split_view(str, ' ');
    REQUIRE(r.size() == 3);
    REQUIRE(r[0] == "hello");
    REQUIRE(r[1] == "world");
    REQUIRE(r[2] == "hi");
}

TEST_CASE("Trim a string", "[string]") {
    std::string_view str = "   hello   ";
    auto r = string::trim<std::string_view, std::string_view>(str);
    REQUIRE(r == "hello");
}
