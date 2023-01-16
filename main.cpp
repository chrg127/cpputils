#include <algorithm>
#include <cassert>
#include <thread>
#include <fmt/core.h>
#include "io.hpp"
#include "cmdline.hpp"
#include "string.hpp"
#include "conf.hpp"
#include "bits.hpp"
#include "random.hpp"
#include "callcommand.hpp"
#include "array.hpp"

int test_cmdline(int argc, char *argv[])
{
    const std::vector<cmdline::Argument> args = {
        { 'h', "help", "print this help text", },
        { 'w', "width", "set width", cmdline::ParamType::Single, "1" },
    };
    if (argc < 2)
        cmdline::print_args(args);
    auto result = cmdline::parse(argc, argv, args);
    if (result.has('h'))
        cmdline::print_args(args);
    if (result.has('w'))
        printf("width = %s\n", result.params['w'].data());
    return 0;
}

int test_string(void)
{
    std::string test = "1,2,3";
    auto r = string::split(test);
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
    auto v = string::split_view(str, ' ');
    for (auto word : v)
        fmt::print("{}\n", word);
}

void test_trim()
{
    std::string_view str = "   hello   ";
    auto res = string::trim<std::string_view, std::string_view>(str);
    fmt::print("{}\n", res);
}

void test_random()
{
    fmt::print("sizeof dist = {}\n", sizeof(std::uniform_int_distribution<int>));
    fmt::print("seed = ");
    for (auto x : rng::seed)
        fmt::print("{} ", x);
    fmt::print("\n");
    for (int i = 0; i < 10; i++)         fmt::print("{} ", rng::get<int>());                 fmt::print("\n");
    for (int i = 0; i < 10; i++)         fmt::print("{} ", rng::get<float>());               fmt::print("\n");
    for (int i = 0; i < 25; i++)         fmt::print("{} ", rng::between(10, 20));            fmt::print("\n");
    for (int i = 0; i < 10; i++)         fmt::print("{} ", rng::between<float>(10, 20));     fmt::print("\n");
    for (int  i = 0; i < 10; i++)        fmt::print("{} ", rng::between<float>(0.0f, 1.0f)); fmt::print("\n");
    fmt::print("hello, {}\n", rng::pick({ "officer", "engineer", "office worker", "judge" }));
}

void test_file_get_line()
{
    auto f = io::File::open("test.txt", io::Access::Read);
    if (!f) {
        fmt::print("[{}] {}\n", f.error().value(), f.error().message());
        return;
    }
    for (std::string line; f.value().get_line(line); )
        fmt::print("line: {}\n", line);
}

void test_error_code()
{
    std::vector<std::thread> ts;
    for (int i = 0; i < 2; i++) {
        ts.push_back(std::thread([i]() {
            auto f = io::read_file(i == 0 ? "main.cpp" : "/never_exists.txt");
            auto err = io::detail::make_error();
            fmt::print("msg = {}\n", err.message());
        }));
    }
    for (auto &t : ts)
        t.join();
}

static std::string parse_error_message(int which, std::string_view name, int num_params)
{
    switch (which) {
    case 0: return fmt::format("Invalid command: {}.", name);
    case 1: return fmt::format("Wrong number of parameters for command {} (got {})", name, num_params);
    default: return "";
    }
}

template <>
int util::try_convert_impl<int>(std::string_view str)
{
    if (auto o = string::to_number(str); o)
        return o.value();
    throw ParseError(fmt::format("invalid number: {}", str));
}

void test_call_command()
{
    std::array<std::string, 3> args = { "1", "2", "3" };
    util::call_command("sum", args, parse_error_message,
        util::Command<int, int, int>{ "sum", "s", [](int a, int b, int c) { fmt::print("{}\n", a+b+c); } },
        util::Command<int, int, int>{ "mul", "s", [](int a, int b, int c) { fmt::print("{}\n", a*b*c); } }
    );
}

void test_2dspan()
{
    auto print = [](auto s) {
        for (auto i = 0u; i < s.height(); i++) {
            for (auto j = 0u; j < s.width(); j++)
                fmt::print("{} ", s[i][j]);
            fmt::print("\n");
        }
    };

    auto v = std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    std::span<int> the_span{v};
    auto s = util::Span2D<int>{v, 4, 4};
    auto sub = s.subspan(1, 1, 3, 3);
    auto sub2 = sub.subspan(0, 0, 2, 2);
    print(s);
    print(sub);
    print(sub2);
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
    // test_random();
    // test_file_get_line();
    // test_error_code();
    // test_call_command();S
    test_2dspan();
    return 0;
}
