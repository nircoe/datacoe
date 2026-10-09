cmake_minimum_required(VERSION 3.22)
project(datacoe VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
include(cmake/datacoe_config.cmake)

option(DATACOE_BUILD_TESTS "Build the datacoe test suite" OFF)

datacoe_fetch_cryptopp()
datacoe_fetch_json()
if(DATACOE_BUILD_TESTS)
    datacoe_fetch_testcoe()
endif()

add_subdirectory(include)
add_subdirectory(src)

if(DATACOE_BUILD_TESTS)
    add_subdirectory(tests)
endif()
