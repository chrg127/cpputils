#include <catch2/catch.hpp>
#include <fmt/core.h>
#include "../src/cmdline.hpp"

const cmdline::Option args[] = {
    { 'h', "help", "print this help text", },
    { 'w', "width", "set width", cmdline::ArgType::Required, "1", "WIDTH" },
    { '\0', "reverse", "reverse stuff", },
};

TEST_CASE("Simple cmdline test", "[cmdline]") {
    char *argv[] = { "programname", "-h", "-w", "3", nullptr };
    int argc = std::size(argv) - 1;
    auto r = cmdline::parse(argc, argv, args);
    REQUIRE(!r.got_error);
    REQUIRE(r.found("help") == true);
    REQUIRE(r.found("width"));
    REQUIRE(r.args["width"] == "3");
    REQUIRE(r.argc == 0);
    REQUIRE(r.argv[0] == nullptr);
}

// TODO: another test for subcommands

// void print_cmdline_result(std::span<const cmdline::Option> opts, cmdline::Result &r)
// {
//     for (auto &opt : opts) {
//         if (r.found(opt.longopt)) {
//             fmt::print("found {}", opt.longopt);
//             if (opt.arg != cmdline::ArgType::None)
//                 fmt::print(", arg = {}", r.args[opt.longopt]);
//             fmt::print("\n");
//         } else {
//             fmt::print("{} not found\n", opt.longopt);
//         }
//     }
// }

// int test_cmdline(int argc, char *argv[])
// {
//     const cmdline::Option args[] = {
//         { 'h', "help", "print this help text", },
//         { 'w', "width", "set width", cmdline::ArgType::Required, "1", "WIDTH" },
//         { '\0', "reverse", "reverse stuff", },
//     };
//     auto r = cmdline::parse(argc, argv, args, cmdline::Flags::StopAtFirstNonOption);
//     if (r.got_error) {
//         cmdline::print_options(args);
//         return 1;
//     }
//     print_cmdline_result(args, r);
//     if (r.found("help")) {
//         cmdline::print_options(args);
//         return 0;
//     }
//     if (r.found("width"))
//         printf("width = %s\n", r.args["width"].data());

//     const cmdline::Option subopts[] = {
//         { 'h', "help", "print help for subcommand" }
//     };

//     if (r.argc > 0) {
//         auto subr = cmdline::parse(r.argc, r.argv, subopts);
//         if (subr.found("help"))
//             cmdline::print_options(subopts);
//         print_cmdline_result(subopts, subr);
//     }
//     return 0;
// }

