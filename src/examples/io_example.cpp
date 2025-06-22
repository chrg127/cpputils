#include <io.hpp>
#include <cstdio>

int main()
{
    printf("home: %s\n", io::directory::home().string().c_str());
    printf("config: %s\n", io::directory::config().string().c_str());
    return 0;
}
