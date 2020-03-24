cmake_minimum_required (VERSION 3.8)

set(SRCS src/entry.cpp
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
target_link_libraries(kernel PRIVATE hal)

if (WIN32)
    set_target_properties(kernel PROPERTIES LINK_FLAGS /SUBSYSTEM:WINDOWS)
    target_link_options(kernel PUBLIC /ENTRY:chinoStartup /NODEFAULTLIB)
endif()
