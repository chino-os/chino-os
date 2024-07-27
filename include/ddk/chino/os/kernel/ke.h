// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "kernel_types.h"
#include "ps.h"
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
};

ps::process &ke_process() noexcept;
bool is_io_worker_thread(ps::thread &thread) noexcept;
} // namespace chino::os::kernel

extern "C" {
[[noreturn]] void ke_startup(const chino::os::kernel::boot_options &options) noexcept;
}
