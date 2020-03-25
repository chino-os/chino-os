project(libfdt)

set(SRCS fdt.c
         fdt_addresses.c
         fdt_empty_tree.c
         fdt_overlay.c
         fdt_ro.c
         fdt_rw.c
         fdt_strerror.c
         fdt_sw.c
         fdt_wip.c)

add_library(fdt STATIC ${SRCS})
target_include_directories(fdt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})