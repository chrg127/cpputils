cmake_minimum_required(VERSION 3.27)

project(cpputils VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(fmt REQUIRED)
find_package(Catch2 3 REQUIRED)

if (MSVC)
    set(warnings /W4 /WX)
else()
    set(warnings -Wall -Wextra -Wpedantic)
endif()

add_executable(io_example src/io.cpp src/examples/io_example.cpp)
target_compile_features(io_example PRIVATE cxx_std_20)
target_compile_options(io_example PRIVATE ${warnings})
target_include_directories(io_example PRIVATE external/include)
target_link_libraries(io_example fmt::fmt)

add_executable(conf_example src/conf.cpp src/io.cpp src/examples/conf_example.cpp)
target_compile_features(conf_example PRIVATE cxx_std_20)
target_compile_options(io_example PRIVATE ${warnings})
target_include_directories(conf_example PRIVATE external/include)
target_link_libraries(conf_example PRIVATE fmt::fmt)

add_executable(cpputils-tester
    src/conf.cpp src/io.cpp
    src/test/array_test.cpp src/test/bits_test.cpp src/test/callcommand_test.cpp
    src/test/cmdline_test.cpp src/test/conf_test.cpp src/test/flags_test.cpp
    src/test/io_test.cpp src/test/io_test.txt src/test/math_test.cpp
    src/test/string_test.cpp
)
target_compile_features(cpputils-tester PRIVATE cxx_std_20)
target_compile_options(io_example PRIVATE ${warnings})
target_include_directories(cpputils-tester PRIVATE external/include)
target_link_libraries(cpputils-tester PRIVATE fmt::fmt Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(cpputils-tester)
