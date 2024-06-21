// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include <chino/os/kernel/hal/cpu/cpu.h>
#include <chino/os/kernel/kernel.h>
#include <chino/version.h>
#include <format>
#include <iostream>
#include <lyra/lyra.hpp>
#include <memory>
#include <stdexcept>
#include <string>

#ifdef WIN32
#include <Windows.h>
#elif defined(__unix__) || defined(__APPLE__)
#include <dlfcn.h>
#endif

using namespace chino::os::hal;
using namespace chino::os::kernel;

#define SYSTEM_PATH "../chino/system/"
#define KERNEL_FILENAME "chino.kernel"
#define KERNEL_STARTUP_STR CHINO_STRINGFY(CHINO_KERNEL_STARTUP)

#ifdef WIN32
#define DYNLIB_PREFIX
#define DYNLIB_EXT ".dll"
#elif defined(__unix__) || defined(__APPLE__)
#define DYNLIB_PREFIX "lib"
#ifdef __unix__
#define DYNLIB_EXT ".so"
#else
#define DYNLIB_EXT ".dylib"
#endif
#endif

#define KERNEL_FILEPATH SYSTEM_PATH DYNLIB_PREFIX KERNEL_FILENAME DYNLIB_EXT

namespace {
using kernel_entry_t = decltype(CHINO_KERNEL_STARTUP) *;

inline constexpr size_t default_memory_size = 1024 * 1024 * 64; // 64MB

kernel_entry_t load_kernel_entry() {
#ifdef WIN32
    auto kernel_lib = LoadLibraryExA(KERNEL_FILEPATH, nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!kernel_lib) {
        throw std::runtime_error(std::format("Unable to load " KERNEL_FILENAME ": {:#08X}", GetLastError()));
    }

    auto kernel_entryp = reinterpret_cast<kernel_entry_t>(GetProcAddress(kernel_lib, KERNEL_STARTUP_STR));
    if (!kernel_entryp) {
        throw std::runtime_error(std::format("Unable to load kernel entrypoint: {:#08X}", GetLastError()));
    }
#else
    auto kernel_lib = dlopen(KERNEL_FILEPATH, RTLD_NOW);
    if (!kernel_lib) {
        throw std::runtime_error(std::string("Unable to load " KERNEL_FILENAME ": ") + dlerror());
    }

    auto kernel_entryp = reinterpret_cast<kernel_entry_t>(dlsym(kernel_lib, KERNEL_STARTUP_STR));
    if (!kernel_entryp) {
        throw std::runtime_error(dlerror());
    }
#endif
    return kernel_entryp;
}
} // namespace

int main(int argc, char **argv) {
    bool show_version = false;
    uint32_t memory_size = default_memory_size;

    auto cli = lyra::cli();
    cli.add_argument(lyra::opt(show_version).name("-v").name("--version").help("show version"));
    cli.add_argument(
        lyra::opt(memory_size, "memory size").optional().name("-m").name("--memory_size").help("set memory size"));

    cli.parse({argc, argv});

    if (show_version) {
        std::cout << "Chino OS emulator " CHINO_VERSION CHINO_VERSION_SUFFIX << " Build " __DATE__ << std::endl
                  << "Copyright (c) SunnyCase." << std::endl;
        return 0;
    }

    // 1. Load kernel
    auto kernel_entryp = load_kernel_entry();

    // 2. Parpare boot options
    // 2.1. Create memory
    auto free_pages = memory_size / cpu_t::min_page_size;
    size_t free_memory_size = cpu_t::min_page_size * (free_pages + 1);
    auto free_memory_holder = std::make_unique<uint8_t[]>(free_memory_size);
    void *free_memory_aligned = free_memory_holder.get();
    if (!std::align(cpu_t::min_page_size, cpu_t::min_page_size * free_pages, free_memory_aligned, free_memory_size)) {
        throw std::runtime_error("Unable to allocate main memory.");
    }

    boot_memory_desc memory_descs[] = {
        {
            .kind = boot_memory_kind::free,
            .physical_address = (uintptr_t)free_memory_aligned,
            .virtual_address = (uintptr_t)free_memory_aligned,
            .size_bytes = free_memory_size,
        },
    };

    // 2.2 Create boot options
    boot_options options{
        .memory_descs = std::span(memory_descs),
    };
    kernel_entryp(options);
}