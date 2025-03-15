cmake_minimum_required(VERSION 3.14)
# cmake_policy(SET CMP0002 NEW)
project(data_management)

set(CMAKE_BUILD_TYPE Debug)
if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /DEBUG")
    set(CMAKE_C_FLAGS_DEBUG "/Zi /DEBUG")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_C_FLAGS_DEBUG "-g")
endif()

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/6910c9d9165801d8827d628cb72eb7ea9dd538c5.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
if(WIN32)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()
FetchContent_MakeAvailable(googletest)

# add_subdirectory(${googletest_SOURCE_DIR})

add_subdirectory(cryptopp-cmake)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

add_subdirectory(tests) # Temporarily remove tests

add_library(${PROJECT_NAME}
    src/game_data.cpp
    src/data_reader_writer.cpp
    src/data_manager.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE cryptopp)

target_include_directories(${PROJECT_NAME} PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/cryptopp-cmake
    ${json_SOURCE_DIR}/single_include/nlohmann)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX)
endif()

# check
message(STATUS "json_SOURCE_DIR: ${json_SOURCE_DIR}")
message(STATUS "googletest_SOURCE_DIR: ${googletest_SOURCE_DIR}") # Verify variable
message(STATUS "cryptopp include dir - ${CMAKE_CURRENT_BINARY_DIR}/cryptopp-cmake/cryptopp")