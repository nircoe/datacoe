function(datacoe_fetch_cryptopp)
    if(TARGET cryptopp)
        message(STATUS "[datacoe] cryptopp already available, skipping fetch")
        return()
    endif()

    message(STATUS "[datacoe] Fetching cryptopp-cmake from source...")

    FetchContent_Declare(
        cryptopp-cmake
        GIT_REPOSITORY https://github.com/abdes/cryptopp-cmake.git
        GIT_TAG CRYPTOPP_8_9_0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(cryptopp-cmake)

    if(MSVC)
        # CryptoPP 8.9.0 fails to build on newer MSVC - integer.cpp and zdeflate.cpp both use a
        # removed helper function for CRYPTOPP_MSC_VERSION >= 1500/1600 with no upper bound.
        # Upstream fixed both by capping them at < 1938; patch it in until they release a
        # version with the fix.
        set(cryptopp_include_dir "${cryptopp-cmake_BINARY_DIR}/${CRYPTOPP_INCLUDE_PREFIX}")

        function(datacoe_patch_cryptopp_msc_guard relative_file old_guard new_guard)
            set(file "${cryptopp_include_dir}/${relative_file}")
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
endfunction()
