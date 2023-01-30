#include "../src/io.hpp"
#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>

TEST_CASE("read_file test", "[io]")
{
    auto text = io::read_file("test/io_test.txt");
    REQUIRE(text.has_value());
    REQUIRE(text.value() == "first line\nsecond line\nthird line\n");
}

TEST_CASE("get_line test", "[io]")
{
    auto f = io::File::open("test/io_test.txt", io::Access::Read);
    REQUIRE(f.has_value());
    std::string line;
    f.value().get_line(line); REQUIRE(line == "first line");
    f.value().get_line(line); REQUIRE(line == "second line");
    f.value().get_line(line); REQUIRE(line == "third line");
}

TEST_CASE("Testing errors when file doesn't exist", "[io]")
{
    auto t = io::read_file("/never_exists.txt");
    REQUIRE(!t.has_value());
    REQUIRE(t.error() == std::errc::no_such_file_or_directory);
}

TEST_CASE("Testing MappedFile", "[io]")
{
    auto f = io::MappedFile::open("test/io_test.txt", io::Access::Read);
    REQUIRE(f.has_value());
    REQUIRE(strcmp((const char *) f.value().data(), "first line\nsecond line\nthird line\n") == 0);
}

