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

// const conf::ValidConfig defaults = {
//     { "a", conf::Value("f") },
//     { "b", conf::Value(1.0f) },
//     { "c", conf::Value(false) },
// };

// void test_conf()
// {
//     const char *path = "conf_test.txt";
//     auto text = io::read_file(path);
//     if (!text) {
//         fmt::print("can't open {}\n", path);
//         return;
//     }
//     std::string errors = "";
//     auto data = conf::parse(text.value(), [&](std::string_view msg) {
//         errors += "    " + std::string(msg) + "\n";
//     });
//     if (!data) {
//         if (!errors.empty())
//             fmt::print("errors on parsing conf file {}:\n{}", path, errors);
//         return;
//     }
//     for (auto [k, v] : data.value())
//         fmt::print("{} = {}\n", k, v.to_string());
//     return;
// }

// void test_conf2()
// {
//     auto conf = conf::parse_or_create("conf_test.txt", defaults, [&](std::string_view msg) {
//         fmt::print("{}\n", msg);
//     });
//     if (!conf)
//         return;
//     for (auto [k, v] : conf.value())
//         fmt::print("{} = {}\n", k, v.to_string());
// }

// void test_find_file(std::string_view name)
// {
//     auto res = conf::find_file(name);
//     if (res)
//         fmt::print("{}\n", res.value().string());
//     else
//         fmt::print("not found: {}\n", name);
// }

