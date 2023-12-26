#include "../src/string.hpp"
#include <catch2/catch_test_macros.hpp>

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

TEST_CASE("Convert number to string", "[string]") {
    int n = 2147483641;
    auto r = string::from_number(n);
    REQUIRE(r.size() == 10);
    REQUIRE(r == "2147483641");
}

TEST_CASE("Convert floating point number to string", "[string]") {
    double n = 0.5;
    auto r = string::from_number(n);
    REQUIRE(r == "0.5");
    n = 0.1 + 0.2;
    r = string::from_number(n);
    REQUIRE(r == "0.30000000000000004");
}

TEST_CASE("Converting string to number", "[string]") {
    auto *c_str = "12345";
    auto str = std::string("12345");
    auto view = std::string_view(str);
    auto n1 = string::to_number(c_str);
    auto n2 = string::to_number(str);
    auto n3 = string::to_number(view);
    REQUIRE(n1.has_value());
    REQUIRE(n2.has_value());
    REQUIRE(n3.has_value());
    REQUIRE(n1 == n2);
    REQUIRE(n1 == n3);
    REQUIRE(n2 == n3);
    REQUIRE(n1.value() == 12345);
}

TEST_CASE("Converting string to floating point number", "[string]") {
    auto *c_str = "0.3";
    auto str = std::string("0.3");
    auto view = std::string_view(str);
    auto n1 = string::to_number<double>(c_str);
    auto n2 = string::to_number<double>(str);
    auto n3 = string::to_number<double>(view);
    REQUIRE(n1.has_value());
    REQUIRE(n2.has_value());
    REQUIRE(n3.has_value());
    REQUIRE(n1 == n2);
    REQUIRE(n1 == n3);
    REQUIRE(n2 == n3);
    REQUIRE(n1.value() == 0.3);
    auto n4 = string::to_number<float>(str);
    REQUIRE(n4.value() == 0.3f);
}
