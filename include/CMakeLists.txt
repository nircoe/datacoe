add_library(datacoe_headers INTERFACE)
target_include_directories(datacoe_headers
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(datacoe_headers
    INTERFACE
        nlohmann_json::nlohmann_json
)
