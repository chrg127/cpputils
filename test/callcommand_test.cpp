#include <catch2/catch.hpp>
#include <fmt/core.h>
#include "../src/callcommand.hpp"

// static std::string parse_error_message(int which, std::string_view name, int num_params)
// {
//     switch (which) {
//     case 0: return fmt::format("Invalid command: {}.", name);
//     case 1: return fmt::format("Wrong number of parameters for command {} (got {})", name, num_params);
//     default: return "";
//     }
// }

// template <>
// int util::try_convert_impl<int>(std::string_view str)
// {
//     if (auto o = string::to_number(str); o)
//         return o.value();
//     throw ParseError(fmt::format("invalid number: {}", str));
// }

// void test_call_command()
// {
//     std::array<std::string, 3> args = { "1", "2", "3" };
//     util::call_command("sum", args, parse_error_message,
//         util::Command<int, int, int>{ "sum", "s", [](int a, int b, int c) { fmt::print("{}\n", a+b+c); } },
//         util::Command<int, int, int>{ "mul", "s", [](int a, int b, int c) { fmt::print("{}\n", a*b*c); } }
//     );
// }

