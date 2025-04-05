cmake_minimum_required(VERSION 3.14)
project(datacoe VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build the test suite" ON)
set(BUILD_TESTS OFF)

# dependencies from external/ (git submodules)
add_subdirectory(external/cryptopp-cmake)
add_subdirectory(external/json)

# project directories
add_subdirectory(include)
add_subdirectory(src)

if(BUILD_TESTS)
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

    add_subdirectory(tests)
endif()

install(
    FILES "${CMAKE_SOURCE_DIR}/external/json/single_include/nlohmann/json.hpp"
    DESTINATION include/nlohmann
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/datacoeconfigversion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR lib)
endif()

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/datacoeconfigversion.cmake"
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/datacoeconfig.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/datacoe"
)

install(
    EXPORT datacoeTargets
    FILE datacoeTargets.cmake
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/datacoe"
)