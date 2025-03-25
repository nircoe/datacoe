cmake_minimum_required(VERSION 3.14)
project(datacoe)

# Debug mode, feel free to modify to your desire
set(CMAKE_BUILD_TYPE Debug)
if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /DEBUG")
    set(CMAKE_C_FLAGS_DEBUG "/Zi /DEBUG")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_C_FLAGS_DEBUG "-g")
endif()

include(FetchContent)

# Currently fetching googletest v1.16.0, 
# Feel free to upgrade and change the URL into the <commit hash>.zip file you would like to upgrade
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/6910c9d9165801d8827d628cb72eb7ea9dd538c5.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
if(WIN32)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()
FetchContent_MakeAvailable(googletest)

add_subdirectory(external/cryptopp-cmake)

# Currently fetching nlohmann/json v3.11.3,
# Feel free to upgrade to your desired version.
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

add_subdirectory(tests)

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

# "Treat warnings as errors" behavior, Feel free to remove/modify it.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX)
endif()