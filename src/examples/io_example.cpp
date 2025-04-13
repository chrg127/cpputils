#include <io.hpp>
#include <fmt/core.h>

int main()
{
    fmt::print("home: {}\n", io::directory::home().string());
    fmt::print("home: {}\n", io::directory::config().string());
    return 0;
}
