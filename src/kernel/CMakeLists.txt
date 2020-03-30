cmake_minimum_required (VERSION 3.8)

set(SRCS src/entry.cpp
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
target_link_libraries(kernel PRIVATE hal fdt drivers ulog)

if (WIN32)
    set_target_properties(kernel PROPERTIES LINK_FLAGS /SUBSYSTEM:WINDOWS)
    target_link_options(kernel PUBLIC /ENTRY:chinoStartup /NODEFAULTLIB)
    target_link_libraries(kernel PRIVATE winres)
endif()
