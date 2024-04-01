#include "../conf.hpp"
#include <fmt/core.h>

using namespace conf::literals;

const conf::Data defaults = {
    { "foo", 1.0_v },
    { "bar", "random_string"_v },
    { "baz", conf::Value(true) },
    { "fooo", 0_v },
    { "barr", conf::Value(conf::ValueList{ 1_v, 2_v, 3_v }) }
};

int main(void)
{
    auto [data, errors] = conf::parse_or_create("app", defaults, { conf::AcceptAnyKey });
    for (auto err : errors)
        fmt::print("{}\n", err.message());

    fmt::print("values found in config:\n");
    for (auto [k, v] : data)
        fmt::print("{} : {}\n", k, v.to_string());

    fmt::print("strings found:\n");
    for (auto s : convert_list_no_errors<std::string>(data["barr"].as<conf::ValueList>()))
        fmt::print("{}\n", s);

    conf::write("app", data);
    return 0;
}

