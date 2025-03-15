# cmake_minimum_required(VERSION 3.14)

# set(GTEST_INCLUDE_DIR "${googletest_SOURCE_DIR}/googletest/include/gtest")

# # Add executable targets
# add_executable(data_manager_tests data_manager_tests.cpp)
# set_target_properties(data_manager_tests PROPERTIES CXX_STANDARD 17)
# target_include_directories(data_manager_tests PRIVATE ${GTEST_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../include)
# target_link_libraries(data_manager_tests PRIVATE data_management gtest_main)
# add_test(NAME data_manager_tests COMMAND data_manager_tests)

# add_executable(data_reader_writer_tests data_reader_writer_tests.cpp)
# set_target_properties(data_reader_writer_tests PROPERTIES CXX_STANDARD 17)
# target_include_directories(data_reader_writer_tests PRIVATE ${GTEST_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../include)
# target_link_libraries(data_reader_writer_tests PRIVATE data_management gtest_main)
# add_test(NAME data_reader_writer_tests COMMAND data_reader_writer_tests)

# add_executable(game_data_tests game_data_tests.cpp)
# set_target_properties(game_data_tests PROPERTIES CXX_STANDARD 17)
# target_include_directories(game_data_tests PRIVATE ${GTEST_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../include)
# target_link_libraries(game_data_tests PRIVATE data_management gtest_main)
# add_test(NAME game_data_tests COMMAND game_data_tests)

cmake_minimum_required(VERSION 3.14)

set(GTEST_INCLUDE_DIR "${googletest_SOURCE_DIR}/googletest/include/gtest")

# Create test targets
set(TEST_FILES
    data_manager_tests.cpp
    data_reader_writer_tests.cpp
    game_data_tests.cpp
    integration_tests.cpp
    performance_tests.cpp
)

# Create individual test executables
foreach(test_file ${TEST_FILES})
    # Extract the base name without extension
    get_filename_component(test_name ${test_file} NAME_WE)
    
    # Add executable target
    add_executable(${test_name} ${test_file})
    
    # Set properties and include directories
    set_target_properties(${test_name} PROPERTIES CXX_STANDARD 17)
    target_include_directories(${test_name} PRIVATE 
        ${GTEST_INCLUDE_DIR} 
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
    )
    
    # Link libraries
    target_link_libraries(${test_name} PRIVATE data_management gtest_main)
    
    # Add as a test
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

# Optional: Add a test coverage target if you're using gcov
if(CMAKE_COMPILER_IS_GNUCXX)
    option(ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" FALSE)
    if(ENABLE_COVERAGE)
        add_compile_options(--coverage -O0)
        add_link_options(--coverage)
    endif()
endif()