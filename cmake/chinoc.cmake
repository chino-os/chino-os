set(CHINOC_PROJ "${TOOLS_DIR}/chinoc")

find_package(Dotnet REQUIRED)

ADD_DOTNET(${CHINOC_PROJ} NETCOREAPP)
set(CHINOC_EXEC ${CMAKE_BINARY_DIR}/chinoc/netcoreapp3.1/chinoc)

function(CHINOC_RENDER Name OUTPUT TEMPLATE)
    message("OUTPUT ${OUTPUT}")
    get_filename_component(OUTPUT_NAME ${OUTPUT} NAME)
    get_filename_component(TEMPLATE_NAME ${TEMPLATE} NAME)
    add_custom_command(OUTPUT ${OUTPUT}
        COMMAND ${CHINOC_EXEC} render --cfg ${CHINO_BOARD_SRC_DIR}/board.json --template ${TEMPLATE} --out ${OUTPUT}
        DEPENDS ${CHINO_BOARD_SRC_DIR}/board.json ${TEMPLATE}
        COMMENT "Render ${TEMPLATE_NAME} to ${OUTPUT_NAME}")
    add_custom_target(${Name} DEPENDS ${OUTPUT})
    set_target_properties(${Name} PROPERTIES TARGET_FILE ${OUTPUT})
endfunction(CHINOC_RENDER)
