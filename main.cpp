// g++ main.cpp conf.cpp -std=c++20 -lfmt

#include <algorithm>
#include <cassert>
#include <fmt/core.h>
#include "io.hpp"
#include "cmdline.hpp"
#include "string.hpp"
#include "conf.hpp"
#include "bits.hpp"
#include "random.hpp"

int test_cmdline(int argc, char *argv[])
{
    const std::vector<cmdline::Argument> args = {
        { 'h', "help", "print this help text", },
        { 'w', "width", "set width", cmdline::ParamType::Single, "1" },
    };
    if (argc < 2)
        cmdline::print_args(args, stderr);
    auto result = cmdline::parse(argc, argv, args);
    if (result.has('h'))
        cmdline::print_args(args, stderr);
    if (result.has('w'))
        printf("width = %s\n", result.params['w'].data());
    return 0;
}

int test_string(void)
{
    std::string test = "1,2,3";
    auto r = str::split(test);
    for (auto sub : r)
        fmt::print("{}\n", sub);
    return 0;
}

int test_read_file(const char *path)
{
    auto text = io::read_file(path);
    if (!text)
        printf("couldn't read file\n");
    else
        printf("%s\n", text.value().data());
    return 0;
}

const conf::ValidConfig valid_conf = {
    { "a", conf::Value("f") },
    { "b", conf::Value(1.0f) },
    { "c", conf::Value(false) },
};

void test_conf()
{
    const char *path = "conf_test.txt";
    auto text = io::read_file(path);
    if (!text) {
        fmt::print("can't open {}\n", path);
        return;
    }
    std::string errors = "";
    auto data = conf::parse(text.value(), valid_conf, [&](std::string_view msg) {
        errors += "    " + std::string(msg) + "\n";
    });
    if (!errors.empty())
        fmt::print("errors on parsing conf file {}:\n{}", path, errors);
    if (!data)
        return;
    for (auto [k, v] : data.value())
        fmt::print("{} = {}\n", k, v.to_string());
    return;
}

void test_find_file(std::string_view name)
{
    auto res = conf::find_file(name);
    if (res)
        fmt::print("{}\n", res.value());
    else
        fmt::print("not found: {}\n", name);
}

void test_bits()
{
    assert(bits::setbit(192, 1, 1) == 194);
}

void test_split()
{
    std::string_view str = "hello world hi";
    auto v = str::split_view(str, ' ');
    for (auto word : v)
        fmt::print("{}\n", word);
}

void test_trim()
{
    std::string_view str = "   hello   ";
    auto res = str::trim<std::string_view, std::string_view>(str);
    fmt::print("{}\n", res);
}

void test_random()
{
    fmt::print("sizeof dist = {}\n", sizeof(std::uniform_int_distribution<int>));
    fmt::print("seed = {}\n", rng::seed);
    for (int i = 0; i < 10; i++)         fmt::print("{} ", rng::get<int>());                 fmt::print("\n");
    for (int i = 0; i < 10; i++)         fmt::print("{} ", rng::get<float>());               fmt::print("\n");
    for (int i = 0; i < 25; i++)         fmt::print("{} ", rng::between(10, 20));            fmt::print("\n");
    for (int i = 0; i < 10; i++)         fmt::print("{} ", rng::between<float>(10, 20));     fmt::print("\n");
    for (int  i = 0; i < 10; i++)        fmt::print("{} ", rng::between<float>(0.0f, 1.0f)); fmt::print("\n");
    for (auto x : rng::shuffle<int>(25)) fmt::print("{} ", x);                               fmt::print("\n");
}

int main(int argc, char *argv[])
{
    // test_cmdline(argc, argv);
    // test_string();
    // test_read_file("main.cpp");
    // test_conf();
    // test_find_file("yanesemu");
    // test_bits();
    // test_split();
    // test_trim();
    test_random();
    return 0;
}
