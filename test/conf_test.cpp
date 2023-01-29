#include "../src/conf.hpp"
#include <catch2/catch.hpp>
#include <fmt/core.h>

const conf::ValidConfig defaults = {
    { "a", conf::Value("f") },
    { "b", conf::Value(1.0f) },
    { "c", conf::Value(false) },
};

TEST_CASE("Simple conf test", "[conf]") {
    auto conf = conf::parse_or_create("conf_test.txt", defaults, [&](std::string_view msg) {
        fmt::print("{}\n", msg);
    });
    REQUIRE(conf != std::nullopt);
    REQUIRE(conf.value()["a"] == std::string("f"));
    REQUIRE(conf.value()["b"] == 1.0f);
    REQUIRE(conf.value()["c"] == false);
}
