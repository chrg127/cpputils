cmake_minimum_required(VERSION 3.27)

project(cpputils VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(CPPUTILS_BUILD_TESTS "Build tests for cpputils." ON)

if (CPPUTILS_BUILD_TESTS)
    find_package(Catch2 3 REQUIRED)
endif()

if (MSVC)
    set(warnings /W4 /WX)
else()
    set(warnings -Wall -Wextra -Wpedantic)
endif()

# example for io library
add_executable(io_example src/io.cpp src/examples/io_example.cpp)
target_compile_features(io_example PRIVATE cxx_std_23)
target_compile_options(io_example PRIVATE ${warnings} -fdiagnostics-all-candidates)
target_include_directories(io_example PRIVATE include)

# example for conf library
add_executable(conf_example src/conf.cpp src/io.cpp src/examples/conf_example.cpp)
target_compile_features(conf_example PRIVATE cxx_std_23)
target_compile_options(conf_example PRIVATE ${warnings})
target_include_directories(conf_example PRIVATE include)

# tests
if (CPPUTILS_BUILD_TESTS)
    add_executable(cpputils-tester
        src/conf.cpp src/io.cpp
        src/test/array_test.cpp src/test/bits_test.cpp src/test/callcommand_test.cpp
        src/test/cmdline_test.cpp src/test/conf_test.cpp src/test/flags_test.cpp
        src/test/io_test.cpp src/test/io_test.txt src/test/math_test.cpp
        src/test/string_test.cpp
    )
    target_compile_features(cpputils-tester PRIVATE cxx_std_23)
    target_compile_options(cpputils-tester PRIVATE ${warnings})
    target_include_directories(cpputils-tester PRIVATE include)
    target_link_libraries(cpputils-tester PRIVATE Catch2::Catch2WithMain)

    include(CTest)
    include(Catch)
    catch_discover_tests(cpputils-tester)
endif()
