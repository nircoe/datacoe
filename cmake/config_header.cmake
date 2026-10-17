function(datacoe_generate_config_header)
    set(DATACOE_CONFIG_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/config")
    set(DATACOE_CONFIG_DIR ${DATACOE_CONFIG_DIR} PARENT_SCOPE)

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/datacoe_config.hpp.in
        ${DATACOE_CONFIG_DIR}/datacoe_config.hpp
        @ONLY
    )
endfunction()
