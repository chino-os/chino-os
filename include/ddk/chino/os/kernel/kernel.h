// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include <cstddef>
#include <cstdint>
#include <span>

#ifdef CHINO_EMULATOR
#define CHINO_KERNEL_API __attribute__((visibility("default")))
#else
#define CHINO_KERNEL_API
#endif

#define CHINO_KERNEL_ENTRY kernel_entry
#define PAGE_SIZE (1024 * 4)

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
    size_t pages;
};

struct boot_context {
    std::span<const boot_memory_desc> memory_descs;
};
} // namespace chino::os::kernel

extern "C" {
[[noreturn]] CHINO_KERNEL_API void CHINO_KERNEL_ENTRY(const chino::os::kernel::boot_context &context);
}
