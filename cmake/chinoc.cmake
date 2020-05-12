set(CHINOC_PROJ "${TOOLS_DIR}/chinoc")

find_package(Dotnet REQUIRED)

BUILD_DOTNET_IMM(${CHINOC_PROJ} NETCOREAPP RELEASE)
set(CHINOC_EXEC ${CMAKE_BINARY_DIR}/chinoc/netcoreapp3.1/chinoc)

function(CHINOC_RENDER Name OUTPUT TEMPLATE)
    message("OUTPUT ${OUTPUT}")
    get_filename_component(OUTPUT_NAME ${OUTPUT} NAME)
    get_filename_component(TEMPLATE_NAME ${TEMPLATE} NAME)
    add_custom_command(OUTPUT ${OUTPUT}
        COMMAND ${CHINOC_EXEC} render --board ${CHINO_BOARD_ID} --template ${TEMPLATE} --out ${OUTPUT}
        DEPENDS ${TEMPLATE}
        COMMENT "Render ${TEMPLATE_NAME} to ${OUTPUT_NAME}")
    add_custom_target(${Name} DEPENDS ${OUTPUT})
    set_target_properties(${Name} PROPERTIES TARGET_FILE ${OUTPUT})
endfunction(CHINOC_RENDER)

function(CHINOC_RENDER_IMM OUTPUT TEMPLATE)
    get_filename_component(OUTPUT_NAME ${OUTPUT} NAME)
    get_filename_component(TEMPLATE_NAME ${TEMPLATE} NAME)
    execute_process(COMMAND ${CHINOC_EXEC} render --board ${CHINO_BOARD_ID} --template ${TEMPLATE} --out ${OUTPUT})
    message("Render ${TEMPLATE_NAME} to ${OUTPUT_NAME}")
endfunction(CHINOC_RENDER_IMM)
