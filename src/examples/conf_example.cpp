#include <cstdio>
#include <conf.hpp>

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
        printf("%s\n", err.message().c_str());

    printf("values found in config:\n");
    for (auto [k, v] : data)
        printf("%s : %s\n", k.c_str(), v.to_string().c_str());

    printf("strings found:\n");
    for (auto s : convert_list_no_errors<std::string>(data["barr"].as<conf::ValueList>()))
        printf("%s\n", s.c_str());

    conf::write("app", data);
    return 0;
}

