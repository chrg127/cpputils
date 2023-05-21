#include "../src/conf.hpp"
#include <fmt/core.h>

const conf::Data defaults = {
    { "foo", conf::Value(1.0f) },
    { "bar", conf::Value("random_string") },
    { "baz", conf::Value(true) },
    { "fooo", conf::Value(0) },
    { "barr", conf::Value(conf::ValueList{}) }
};

int main(void)
{
    auto res = conf::parse_or_create("app", defaults);
    if (!res) {
        for (auto err : res.error())
            fmt::print("{}\n", err.message());
        return 1;
    }

    auto &conf    = res.value();
    auto warnings = conf::validate(conf, defaults);
    for (auto &w : warnings)
        fmt::print("warning: {}\n", w.message());

    fmt::print("values found in config:\n");
    for (auto [k, v] : conf)
        fmt::print("{} : {}\n", k, v.to_string());

    return 0;
}
