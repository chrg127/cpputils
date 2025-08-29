#include <catch2/catch_test_macros.hpp>
#include <conf.hpp>

const conf::Data defaults = {
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
    auto [conf, errors] = conf::parse(text, {}, { conf::AcceptAnyKey });
    REQUIRE(conf["a"] == std::string("f"));
    REQUIRE(conf["b"] == 1.0f);
    REQUIRE(conf["c"] == false);
}
