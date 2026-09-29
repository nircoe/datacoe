cmake_minimum_required(VERSION 3.22)
project(datacoe VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build the test suite" ON)

# dependencies from external/ (git submodules)
add_subdirectory(external/cryptopp-cmake)
add_subdirectory(external/json)

if(MSVC)
    # CryptoPP 8.9.0 fails to build on newer MSVC - integer.cpp and zdeflate.cpp both use a
    # removed helper function for CRYPTOPP_MSC_VERSION >= 1500/1600 with no upper bound. Upstream
    # fixed both by capping them at < 1938; patch it in until they release a version with the fix.
    function(datacoe_patch_cryptopp_msc_guard relative_file old_guard new_guard)
        set(file "${CMAKE_CURRENT_BINARY_DIR}/external/cryptopp-cmake/${CRYPTOPP_INCLUDE_PREFIX}/${relative_file}")
        if(EXISTS "${file}")
            file(READ "${file}" contents)
            string(REPLACE "${old_guard}" "${new_guard}" contents "${contents}")
            file(WRITE "${file}" "${contents}")
        endif()
    endfunction()

    datacoe_patch_cryptopp_msc_guard(integer.cpp
        "#if (CRYPTOPP_MSC_VERSION >= 1500)"
        "#if (CRYPTOPP_MSC_VERSION >= 1500) && (CRYPTOPP_MSC_VERSION < 1938)")
    datacoe_patch_cryptopp_msc_guard(zdeflate.cpp
        "#if CRYPTOPP_MSC_VERSION >= 1600"
        "#if (CRYPTOPP_MSC_VERSION >= 1600) && (CRYPTOPP_MSC_VERSION < 1938)")
endif()

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