// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
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

struct boot_context {
    std::span<const boot_memory_desc> memory_descs;
};
} // namespace chino::os::kernel

extern "C" {
[[noreturn]] CHINO_KERNEL_API void CHINO_KERNEL_STARTUP(const chino::os::kernel::boot_context &context);
}
