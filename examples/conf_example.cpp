#include "../src/conf.hpp"
#include <fmt/core.h>

using namespace conf::literals;

const conf::Data defaults = {
    { "foo", 1.0_v },
    { "bar", "random_string"_v },
    { "baz", conf::Value(true) },
    { "fooo", 0_v },
    { "barr", conf::Value(conf::ValueList{ 1_v, 2_v, 3_v }) }
};

std::vector<std::string> getstrings(conf::Value list)
{
    std::vector<std::string> strings;
    for (auto v : list.as<conf::ValueList>()) {
        if (v.type() == conf::Type::String)
            strings.push_back(v.as<std::string>());
    }
    return strings;
}

int main(void)
{
    auto [data, errors] = conf::parse_or_create("app", defaults, { conf::AcceptAnyKey });
    for (auto err : errors)
        fmt::print("{}\n", err.message());

    fmt::print("values found in config:\n");
    for (auto [k, v] : data)
        fmt::print("{} : {}\n", k, v.to_string());

    fmt::print("strings found:\n");
    for (auto s : getstrings(data["barr"]))
        fmt::print("{}\n", s);

    conf::write("app", data);
    return 0;
}

