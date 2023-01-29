#include <catch2/catch.hpp>
#include <fmt/core.h>
#include "../src/io.hpp"

namespace fs = std::filesystem;

// int test_read_file(const char *path)
// {
//     auto text = io::read_file(path);
//     if (!text)
//         printf("couldn't read file\n");
//     else
//         printf("%s\n", text.value().data());
//     return 0;
// }

// void test_file_get_line()
// {
//     auto f = io::File::open("test.txt", io::Access::Read);
//     if (!f) {
//         fmt::print("[{}] {}\n", f.error().value(), f.error().message());
//         return;
//     }
//     for (std::string line; f.value().get_line(line); )
//         fmt::print("line: {}\n", line);
// }

// void test_error_code()
// {
//     std::vector<std::thread> ts;
//     for (int i = 0; i < 2; i++) {
//         ts.push_back(std::thread([i]() {
//             auto f = io::read_file(i == 0 ? "main.cpp" : "/never_exists.txt");
//             auto err = io::detail::make_error();
//             fmt::print("msg = {}\n", err.message());
//         }));
//     }
//     for (auto &t : ts)
//         t.join();
// }

// void test_mapped_file()
// {
//     auto f = io::MappedFile::open(fs::path("concepts.hpp"), io::Access::Read);
//     if (!f) {
//         fmt::print("[{}] {}\n", f.error().value(), f.error().message());
//         return;
//     }
//     for (auto c : f.value())
//         fmt::print("{}", (char) c);
// }

