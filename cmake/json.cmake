function(datacoe_fetch_json)
    if(TARGET nlohmann_json::nlohmann_json)
        message(STATUS "[datacoe] nlohmann_json already available, skipping fetch")
        return()
    endif()

    message(STATUS "[datacoe] Fetching json from source...")

    FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(json)
endfunction()
