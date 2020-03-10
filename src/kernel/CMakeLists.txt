cmake_minimum_required (VERSION 3.8)

set(SRCS src/entry.cpp
         src/object/object.cpp
         src/threading/process.cpp
         src/threading/scheduler.cpp
         src/threading/synchronize.cpp)

add_executable(kernel ${SRCS})
target_include_directories(kernel PUBLIC include)
target_link_libraries(kernel PUBLIC api)
target_link_libraries(kernel PRIVATE hal)

if (WIN32)
    set_target_properties(kernel PROPERTIES LINK_FLAGS /SUBSYSTEM:WINDOWS)
endif()
