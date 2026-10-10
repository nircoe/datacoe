enable_testing()

datacoe_ignore_external_warnings(testcoe_headers)

set(TEST_FILES
    data_manager_tests.cpp
    data_reader_writer_tests.cpp
    game_data_tests.cpp
    integration_tests.cpp
    performance_tests.cpp
    memory_tests.cpp
    error_handling_tests.cpp
    saveable_tests.cpp
    utils/io_tests.cpp
)

add_executable(datacoe_tests
    main.cpp
    ${TEST_FILES}
)

target_link_libraries(datacoe_tests
    PRIVATE
        datacoe
        testcoe
)

target_include_directories(datacoe_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

datacoe_enable_strict_warnings(datacoe_tests)
datacoe_enable_static_mingw_runtime(datacoe_tests)

if(MSVC)
    # GTest macros produce symbol names that overflow MSVC's decorated name limit (C4503)
    target_compile_options(datacoe_tests PRIVATE /wd4503)
endif()

add_test(NAME datacoe_tests COMMAND datacoe_tests)

option(DATACOE_BUILD_INDIVIDUAL_TESTS "Build individual test executables" OFF)
if(DATACOE_BUILD_INDIVIDUAL_TESTS)
    foreach(test_file ${TEST_FILES})
        get_filename_component(test_name ${test_file} NAME_WE)

        add_executable(${test_name} ${test_file})

        target_link_libraries(${test_name}
            PRIVATE
                datacoe
                testcoe
        )

        target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

        datacoe_enable_strict_warnings(${test_name})
        datacoe_enable_static_mingw_runtime(${test_name})

        if(MSVC)
            target_compile_options(${test_name} PRIVATE /wd4503)
        endif()

        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()
endif()
