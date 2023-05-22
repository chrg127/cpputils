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
    auto [data, errors] = conf::parse_or_create("app", defaults);
    for (auto err : errors)
        fmt::print("{}\n", err.message());

    fmt::print("values found in config:\n");
    for (auto [k, v] : data)
        fmt::print("{} : {}\n", k, v.to_string());

    return 0;
}
