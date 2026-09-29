function(fetch_testcoe)
    message(STATUS "[datacoe] Fetching testcoe from source...")

    FetchContent_Declare(
        testcoe
        GIT_REPOSITORY https://github.com/nircoe/testcoe.git
        GIT_TAG v0.1.2
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(testcoe)
endfunction()
