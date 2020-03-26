set(DTC_EXECUTABLE "${TOOLS_DIR}/win32/dtc.exe")

function(DTC_COMPILE_DTB Name)
  set(DTC_OUTPUTS)
  foreach(FILE ${ARGN})
    get_filename_component(DTC_OUTPUT ${FILE} NAME_WE)
    set(DTC_OUTPUT
      "${CMAKE_CURRENT_BINARY_DIR}/${DTC_OUTPUT}.dtb")
    list(APPEND DTC_OUTPUTS ${DTC_OUTPUT})
  
    add_custom_command(OUTPUT ${DTC_OUTPUT}
      COMMAND ${DTC_EXECUTABLE}
      ARGS -O dtb -o ${DTC_OUTPUT} ${FILE} -i ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${FILE}
      COMMENT "Compiling ${FILE} to dtb"
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  endforeach()
  add_custom_target(${Name} DEPENDS ${DTC_OUTPUTS})
endfunction()
