cmake_minimum_required (VERSION 3.13)

if (CHINO_DRV_WIN32_NETIF)
    target_sources(drivers PRIVATE win32-netif/win32-netif.cpp)
    target_link_libraries(drivers PRIVATE CONAN_PKG::winpcap)
endif()
