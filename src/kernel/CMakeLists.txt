cmake_minimum_required (VERSION 3.13)

add_subdirectory(src/crt)

set(SRCS src/entry.cpp
         src/crt/except.cpp
         src/diagnostic/logging.cpp
         src/io/device.cpp
         src/io/device_descriptor.cpp
         src/io/file.cpp
         src/io/io_manager.cpp
         src/memory/heap_allocator.cpp
         src/memory/memory_manager.cpp
         src/object/object.cpp
         src/object/directory.cpp
         src/object/handle_table.cpp
         src/threading/process.cpp
         src/threading/scheduler.cpp
         src/threading/synchronize.cpp)

add_executable(kernel ${SRCS})
target_include_directories(kernel PUBLIC include)
target_link_libraries(kernel PUBLIC api ddk)
target_link_libraries(kernel PRIVATE fdt ulog shell)

if (WIN32)
    set_target_properties(kernel PROPERTIES LINK_FLAGS /SUBSYSTEM:WINDOWS)
    target_link_options(kernel PUBLIC /ENTRY:chinoStartup /NODEFAULTLIB)
    target_link_libraries(kernel PRIVATE winres)
    target_link_libraries(kernel PRIVATE hal drivers)
else()
    target_link_options(kernel PUBLIC -Wl,--gc-sections -T ${CHINO_KERNEL_LDS} -fuse-ld=bfd)
    target_compile_options(kernel PRIVATE -nostartfiles)
    target_link_libraries(kernel PRIVATE c stdc++ crt)
    target_link_libraries(kernel PRIVATE -Wl,--whole-archive hal drivers -Wl,--no-whole-archive)
    
    set(CHINO_BIN_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/chino.bin)
    add_custom_command(OUTPUT ${CHINO_BIN_PATH}
        COMMAND rm -f ${CHINO_BIN_PATH}
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:kernel> ${CHINO_BIN_PATH}
        DEPENDS kernel
        COMMENT "Generating chino.bin ...")

    add_custom_target(firmware DEPENDS ${CHINO_BIN_PATH})
endif()
