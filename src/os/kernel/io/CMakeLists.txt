cmake_minimum_required(VERSION 3.13)

set(SRCS device.cpp
         file.cpp
         iocp.cpp
         io_manager.cpp
         io_request.cpp
         irq_handler.cpp
)

target_sources(chino.kernel PRIVATE ${SRCS})
target_link_libraries(chino.kernel PRIVATE chino.drivers)
