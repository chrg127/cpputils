#include "../src/conf.hpp"
#include <fmt/core.h>

const conf::Data defaults = {
    { "foo", conf::Value(1.0f) },
    { "bar", conf::Value("random_string") },
    { "baz", conf::Value(true) },
    { "fooo", conf::Value(0) }
};

int main(void)
{
    auto res = conf::parse_or_create("app.conf", defaults);
    if (!res) {
        for (auto err : res.error())
            fmt::print("{}\n", conf::default_message(err));
        return 1;
    }
    auto &conf = res.value();

    auto warnings = conf::validate(conf, defaults);
    for (auto &w : warnings)
        fmt::print("warning: {}\n", conf::default_message(w));
    // can also return if warnings.size() > 0

    fmt::print("values found in config:\n");
    for (auto [k, v] : conf)
        fmt::print("{} => {}\n", k, v.to_string());
    return 0;
}