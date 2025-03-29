add_library(datacoe_headers INTERFACE)
target_include_directories(datacoe_headers
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/external/json/single_include>
        $<INSTALL_INTERFACE:include>
)   

install(
    DIRECTORY datacoe
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

install(
    TARGETS datacoe_headers
    EXPORT datacoeTargets
)