// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <bit>
#include <chino/os/kernel/kernel_types.h>
#include <chino/os/processapi.h>
#include <chino/units.h>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <span>

#if defined(__x86_64__)
#include <xmmintrin.h>
#endif

namespace chino::os::kernel::hal {
struct alignas(16) emulator_thread_context {
#if defined(__x86_64__)
    uintptr_t rax;
    uintptr_t rbx;
    uintptr_t rcx;
    uintptr_t rdx;
    uintptr_t rsp;
    uintptr_t rbp;
    uintptr_t rsi;
    uintptr_t rdi;
    uintptr_t r8;
    uintptr_t r9;
    uintptr_t r10;
    uintptr_t r11;
    uintptr_t r12;
    uintptr_t r13;
    uintptr_t r14;
    uintptr_t r15;

    uintptr_t rip;

    __m128 xmm[16];
#endif
};

enum class emulator_irq_number {
    system_tick,
};

using arch_thread_context_t = emulator_thread_context;
using arch_irq_state_t = uint32_t;
using arch_irq_number_t = emulator_irq_number;

class emulator_arch {
  public:
    inline static constexpr size_t min_page_size = 4 * KiB;

    static void arch_startup(size_t memory_size);

    static constexpr size_t current_cpu_id() noexcept { return 0; }
    static std::chrono::milliseconds current_cpu_time() noexcept;

    static emulator_thread_context initialize_thread_context(std::span<std::byte> stack,
                                                             ps::thread_main_thunk_t thread_thunk, void *thread,
                                                             thread_start_t entry_point, void *entry_arg) noexcept;

    [[noreturn]] static void restore_context(emulator_thread_context &context) noexcept;

    static arch_irq_state_t disable_irq() noexcept;
    static void restore_irq(arch_irq_state_t state) noexcept;

    static void enable_system_tick(std::chrono::milliseconds due_time) noexcept;
};

using arch_t = emulator_arch;
} // namespace chino::os::kernel::hal
