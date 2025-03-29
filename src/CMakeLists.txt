add_library(datacoe
    data_manager.cpp
    data_reader_writer.cpp
    game_data.cpp
)

target_link_libraries(datacoe
    PRIVATE
        cryptopp
    PUBLIC
        datacoe_headers
)

target_include_directories(datacoe
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(datacoe PRIVATE -Wall -Wextra -Werror)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(datacoe PRIVATE /W3 /WX)
endif()

install(
    TARGETS datacoe
    EXPORT datacoeTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)