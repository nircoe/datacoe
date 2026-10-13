add_library(datacoe
    datacoe/utils/io.cpp
)

target_link_libraries(datacoe
    PRIVATE
        cryptopp
    PUBLIC
        datacoe_headers
)

datacoe_enable_strict_warnings(datacoe)

if(MINGW)
    target_link_options(datacoe INTERFACE -static-libgcc -static-libstdc++ -static)
endif()
