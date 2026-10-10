add_library(datacoe
    data_manager.cpp
    data_reader_writer.cpp
    game_data.cpp
    datacoe/utils/io.cpp
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

datacoe_enable_strict_warnings(datacoe)

if(MINGW)
    target_link_options(datacoe INTERFACE -static-libgcc -static-libstdc++ -static)
endif()
