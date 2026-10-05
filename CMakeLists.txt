cmake_minimum_required(VERSION 3.22)
project(datacoe VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/datacoe_config.cmake)

# dependencies
include(FetchContent)
datacoe_fetch_cryptopp()
datacoe_fetch_json()
datacoe_fetch_testcoe()

# project directories
add_subdirectory(include)
add_subdirectory(src)

if(DATACOE_BUILD_TESTS)
    add_subdirectory(tests)
endif()
