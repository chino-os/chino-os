// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include <chino/os/kernel/kernel.h>
#include <chino/version.h>
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

using namespace chino::os::kernel;

#define STR_(x) #x
#define STR(x) STR_(x)

#define SYSTEM_PATH "../chino/system/"
#define KERNEL_FILENAME "libchino.kernel.dylib"
#define KERNEL_ENTRY_STR STR(CHINO_KERNEL_ENTRY)

namespace {
using kernel_entry_t = decltype(CHINO_KERNEL_ENTRY) *;

inline constexpr size_t default_memory_size = 1024 * 1024 * 64; // 64MB

kernel_entry_t load_kernel_entry() {
    auto kernel_lib = dlopen(SYSTEM_PATH KERNEL_FILENAME, RTLD_NOW);
    if (!kernel_lib) {
        throw std::runtime_error(std::string("Unable to load " KERNEL_FILENAME ": ") + dlerror());
    }

    auto kernel_entryp = reinterpret_cast<decltype(CHINO_KERNEL_ENTRY) *>(dlsym(kernel_lib, KERNEL_ENTRY_STR));
    if (!kernel_entryp) {
        throw std::runtime_error(dlerror());
    }
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

    // 2. Parpare boot context
    // 2.1. Create memory
    auto free_pages = memory_size / PAGE_SIZE * PAGE_SIZE;
    size_t free_memory_size = PAGE_SIZE * (free_pages + 1);
    auto free_memory_holder = std::make_unique<uint8_t[]>(free_memory_size);
    void *free_memory_aligned = free_memory_holder.get();
    if (!std::align(PAGE_SIZE, PAGE_SIZE * free_pages, free_memory_aligned, free_memory_size)) {
        throw std::runtime_error("Unable to allocate main memory.");
    }

    boot_memory_desc memory_descs[] = {
        {
            .kind = boot_memory_kind::free,
            .physical_address = (uintptr_t)free_memory_aligned,
            .virtual_address = (uintptr_t)free_memory_aligned,
            .pages = free_pages,
        },
    };

    // 2.2 Create context
    boot_context context{
        .memory_descs = std::span(memory_descs),
    };
    kernel_entryp(context);
}
