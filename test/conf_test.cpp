#include "../src/conf.hpp"
#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>

const std::map<std::string, conf::Value> defaults = {
    { "a", conf::Value("f") },
    { "b", conf::Value(1.0f) },
    { "c", conf::Value(false) },
};

TEST_CASE("Simple conf test", "[conf]") {
    auto text = R"(
        a = "f"
        b = 1.0
        c = false
    )";
    auto res = conf::parse(text);
    REQUIRE(bool(res));
    auto &conf = res.value();
    REQUIRE(conf["a"] == std::string("f"));
    REQUIRE(conf["b"] == 1.0f);
    REQUIRE(conf["c"] == false);
}
