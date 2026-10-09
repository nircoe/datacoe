function(datacoe_fetch_testcoe)
    if(TARGET testcoe)
        message(STATUS "[datacoe] testcoe already available, skipping fetch")
        return()
    endif()

    message(STATUS "[datacoe] Fetching testcoe from source...")
    message(STATUS "[datacoe] To change: \"set(DATACOE_BUILD_TESTS OFF)\" before fetching datacoe")

    FetchContent_Declare(
        testcoe
        GIT_REPOSITORY https://github.com/nircoe/testcoe.git
        GIT_TAG v0.2.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(testcoe)
endfunction()
