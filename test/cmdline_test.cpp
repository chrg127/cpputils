#include "../src/cmdline.hpp"
#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>

const cmdline::Option args[] = {
    { 'h', "help", "print this help text", },
    { 'w', "width", "set width", cmdline::ArgType::Required, "1", "WIDTH" },
    { '\0', "reverse", "reverse stuff", },
};

const cmdline::Option subopts[] = {
    { 'h', "help", "print help for subcommand" }
};

TEST_CASE("Simple cmdline test", "[cmdline]") {
    const char *argv[] = { "programname", "-h", "-w", "3", nullptr };
    int argc = std::size(argv) - 1;
    auto r = cmdline::parse(argc, argv, args);
    REQUIRE(!r.got_error);
    REQUIRE(r.found("help") == true);
    REQUIRE(r.found("width"));
    REQUIRE(r.args["width"] == "3");
    REQUIRE(r.argc == 0);
    REQUIRE(r.argv[0] == nullptr);
}

TEST_CASE("Subcommands test", "[cmdline]")
{
    const char *argv[] = { "programname", "-h", "subcmd", "-h", nullptr};
    int argc = std::size(argv) - 1;
    auto r = cmdline::parse(argc, argv, args, { cmdline::StopAtFirstNonOption });
    REQUIRE(!r.got_error);
    REQUIRE(r.found("help"));
    REQUIRE(!r.found("width"));
    REQUIRE(r.argc == 2);
    REQUIRE(r.argv[0] == std::string_view("subcmd"));

    auto rsub = cmdline::parse(r.argc, r.argv, subopts);
    REQUIRE(!rsub.got_error);
    REQUIRE(rsub.found("help"));
}

