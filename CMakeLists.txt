cmake_minimum_required(VERSION 3.22)
project(datacoe VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build the test suite" ON)

include(cmake/datacoe_config.cmake)

# dependencies
include(FetchContent)
fetch_cryptopp()
fetch_json()

# project directories
add_subdirectory(include)
add_subdirectory(src)

if(BUILD_TESTS)
    fetch_testcoe()

    add_subdirectory(tests)
endif()
