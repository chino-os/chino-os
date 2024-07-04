// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "threading.h"
#include <cstddef>
#include <cstdint>
#include <span>

#ifdef CHINO_EMULATOR
#ifdef WIN32
#define CHINO_KERNEL_API __declspec(dllexport)
#else
#define CHINO_KERNEL_API __attribute__((visibility("default")))
#endif
#else
#define CHINO_KERNEL_API
#endif

#define CHINO_KERNEL_STARTUP ke_startup
#ifdef CHINO_EMULATOR
namespace chino::os::kernel::hal {
struct emulator_method_table;
}
#endif

namespace chino::os::kernel {

enum class boot_memory_kind {
    free,
    boot,
    kernel,
    reserved,
};

struct boot_memory_desc {
    boot_memory_kind kind;
    uintptr_t physical_address;
    uintptr_t virtual_address;
    size_t size_bytes;
};

struct boot_options {
    std::span<const boot_memory_desc> memory_descs;
#ifdef CHINO_EMULATOR
    hal::emulator_method_table *emu_mt;
#endif
};

typedef void (*thread_main_thunk_t)(void *thread, ps::thread_start_t entry_point, void *entry_arg);
} // namespace chino::os::kernel

extern "C" {
[[noreturn]] CHINO_KERNEL_API void CHINO_KERNEL_STARTUP(const chino::os::kernel::boot_options &options);
}
