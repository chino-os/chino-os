cmake_minimum_required (VERSION 3.13)

if (CHINO_DRV_WIN32_FS)
    target_sources(drivers PRIVATE win32-fs/win32-fs.cpp)
endif()
