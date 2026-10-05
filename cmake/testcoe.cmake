function(datacoe_fetch_testcoe)
    if(NOT DEFINED DATACOE_BUILD_TESTS)
        set(DATACOE_BUILD_TESTS OFF)
    endif()

    if(DATACOE_BUILD_TESTS)
        message(STATUS "[datacoe] Building tests with testcoe")
        message(STATUS "[datacoe] To disable: \"set(DATACOE_BUILD_TESTS OFF)\" before fetching datacoe")
        set(DATACOE_BUILD_TESTS 1 PARENT_SCOPE)

        message(STATUS "[datacoe] Fetching testcoe from source...")

        FetchContent_Declare(
            testcoe
            GIT_REPOSITORY https://github.com/nircoe/testcoe.git
            GIT_TAG v0.1.2
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(testcoe)
    else()
        message(STATUS "[datacoe] Tests disabled")
        message(STATUS "[datacoe] To enable: \"set(DATACOE_BUILD_TESTS ON)\" before fetching datacoe")
        set(DATACOE_BUILD_TESTS 0 PARENT_SCOPE)
    endif()
endfunction()
