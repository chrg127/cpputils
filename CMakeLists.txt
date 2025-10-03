cmake_minimum_required(VERSION 3.27)

project(cpputils VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(CPPUTILS_BUILD_TESTS "Build tests for cpputils." ON)

if (CPPUTILS_BUILD_TESTS)
    find_package(Catch2 3 REQUIRED)
endif()

# example for io library
add_executable(io_example src/io.cpp src/examples/io_example.cpp)
target_compile_features(io_example PRIVATE cxx_std_23)
target_compile_options(io_example PRIVATE -Wall -Wextra -Wpedantic -fdiagnostics-all-candidates)
target_include_directories(io_example PRIVATE include)

# example for conf library
add_executable(conf_example src/conf.cpp src/io.cpp src/examples/conf_example.cpp)
target_compile_features(conf_example PRIVATE cxx_std_23)
target_compile_options(conf_example PRIVATE -Wall -Wextra -Wpedantic -fdiagnostics-all-candidates)
target_include_directories(conf_example PRIVATE include)

# tests
if (CPPUTILS_BUILD_TESTS)
    file(GLOB test_files "src/test/*.cpp")
    add_executable(cpputils-tester
        src/conf.cpp src/io.cpp
        ${test_files}
        src/test/io_test.txt
    )
    target_compile_features(cpputils-tester PRIVATE cxx_std_23)
    target_compile_options(cpputils-tester PRIVATE -Wall -Wextra -Wpedantic)
    target_include_directories(cpputils-tester PRIVATE include)
    target_link_libraries(cpputils-tester PRIVATE Catch2::Catch2WithMain)

    include(CTest)
    include(Catch)
    catch_discover_tests(cpputils-tester)
endif()
