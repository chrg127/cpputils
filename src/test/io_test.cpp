#include "../io.hpp"
#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>

TEST_CASE("read_file test", "[io]")
{
    auto text = io::read_file("src/test/io_test.txt");
    REQUIRE(text.has_value());
    REQUIRE(text.value() == "first line\nsecond line\nthird line\n");
}

TEST_CASE("get_line test", "[io]")
{
    auto f = io::File::open("src/test/io_test.txt", io::Access::Read);
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
    auto f = io::MappedFile::open("src/test/io_test.txt", io::Access::Read);
    REQUIRE(f.has_value());
    REQUIRE(strcmp((const char *) f.value().data(), "first line\nsecond line\nthird line\n") == 0);
}

TEST_CASE("Reading bytes with io::File", "[io]")
{
    auto f = io::File::open("src/test/io_test.txt", io::Access::Read);
    REQUIRE(f.has_value());
    auto b1 = f.value().read(5);
    auto correct_b1 = std::vector<u8>{'f', 'i', 'r', 's', 't'};
    REQUIRE(b1 == correct_b1);
}

TEST_CASE("Writing bytes with io::File", "[io]")
{
    auto f = io::File::open("src/test/io_write_test.txt", io::Access::Write);
    REQUIRE(f.has_value());
    auto b = std::vector<u8>{0, 1, 2, 3, 4, 5};
    auto nread = f.value().write(b);
    REQUIRE(nread == b.size());
    std::remove("src/test/io_write_test.txt");
}
