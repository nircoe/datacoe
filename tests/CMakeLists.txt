enable_testing()

set(TEST_FILES
    data_manager_tests.cpp
    data_reader_writer_tests.cpp
    game_data_tests.cpp
    integration_tests.cpp
    performance_tests.cpp
    memory_tests.cpp
    error_handling_tests.cpp
)

add_executable(all_tests
    main.cpp
    ${TEST_FILES}
)

set_target_properties(all_tests PROPERTIES CXX_STANDARD 23)

target_link_libraries(all_tests PRIVATE
    datacoe
    testcoe
)


if(MSVC)
    # MSVC-specific settings treat warnings as errors
    target_compile_options(all_tests PRIVATE /W4 /WX)
    # Disable warning about truncated symbol names for MSVC (common with GTest macros)
    target_compile_options(all_tests PRIVATE /wd4503)
else()
    # GCC/Clang settings treat warnings as errors
    target_compile_options(all_tests PRIVATE -Wall -Wextra -Werror)
endif()

# Register the unified test
add_test(NAME all_tests COMMAND all_tests)

# Optional: Create individual test executables
option(BUILD_INDIVIDUAL_TESTS "Build individual test executables" OFF)
if(BUILD_INDIVIDUAL_TESTS)
    foreach(test_file ${TEST_FILES})
        # Extract the base name without extension
        get_filename_component(test_name ${test_file} NAME_WE)

        add_executable(${test_name} ${test_file})

        # Set properties and include directories
        set_target_properties(${test_name} PROPERTIES CXX_STANDARD 23)

        # Link libraries
        target_link_libraries(${test_name} PRIVATE
            datacoe
            testcoe
        )

        # Platform-specific settings treat warnings as errors
        if(MSVC)
            target_compile_options(${test_name} PRIVATE /W4 /WX /wd4503)
        else()
            target_compile_options(${test_name} PRIVATE -Wall -Wextra -Werror)
        endif()

        # Add as a test
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()
endif()
