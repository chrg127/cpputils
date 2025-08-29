#include <catch2/catch_test_macros.hpp>
#include <callcommand.hpp>
#include <string.hpp>

static std::string parse_error_message(int which, std::string_view name, int num_params)
{
    switch (which) {
    case 0: return std::format("Invalid command: {}.", name);
    case 1: return std::format("Wrong number of parameters for command {} (got {})", name, num_params);
    default: return "";
    }
}

template <>
int util::try_convert_impl<int>(std::string_view str)
{
    if (auto o = string::to_number(str); o)
        return o.value();
    throw ParseError(std::format("invalid number: {}", str));
}

int call_command(std::string_view which, std::array<std::string, 3> args)
{
    int result;
    util::call_command(which, args, parse_error_message,
        util::Command<int, int, int>{ "sum", "s", [&](int a, int b, int c) { result = a+b+c; } },
        util::Command<int, int, int>{ "mul", "s", [&](int a, int b, int c) { result = a*b*c; } }
    );
    return result;
}

TEST_CASE("Simple commands", "[callcommand]")
{
    REQUIRE(call_command("sum", { "1", "2", "3" }) == 6);
    REQUIRE(call_command("mul", { "2", "2", "3" }) == 12);
}

