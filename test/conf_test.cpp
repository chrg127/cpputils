#include <catch2/catch.hpp>
#include <fmt/core.h>
#include "../src/conf.hpp"

const conf::ValidConfig defaults = {
    { "a", conf::Value("f") },
    { "b", conf::Value(1.0f) },
    { "c", conf::Value(false) },
};

TEST_CASE("Simple conf test", "[conf]") {
    auto conf = conf::parse_or_create("conf_test.txt", defaults, [&](std::string_view msg) {
        fmt::print("{}\n", msg);
    });
    if (!conf)
        return;
    REQUIRE(conf.value()["a"] == std::string("f"));
}
