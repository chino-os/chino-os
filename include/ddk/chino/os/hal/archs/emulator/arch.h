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
using arch_irq_state_t = uint32_t;

enum class emulator_irq_number {
    system_tick,
};

using arch_irq_number_t = emulator_irq_number;

class emulator_arch {
  public:
    inline static constexpr size_t min_page_size = 4 * KiB;

    static void arch_startup(size_t memory_size);

    static constexpr size_t current_cpu_id() noexcept { return 0; }
    static std::chrono::milliseconds current_cpu_time() noexcept;

    static void initialize_thread_stack(uintptr_t *&stack_top) noexcept {}

    static void yield_cpu() noexcept;
    static void yield_context(ps::thread &old_thread, ps::thread &new_thread, bool scheduled) noexcept;
    [[noreturn]] static void start_schedule(ps::thread &thread) noexcept;

    static void enable_irq() noexcept;
    static arch_irq_state_t disable_irq() noexcept;
    static bool restore_irq(arch_irq_state_t state) noexcept;

    static void enable_system_tick(std::chrono::milliseconds due_time) noexcept;
};

using arch_t = emulator_arch;
} // namespace chino::os::kernel::hal
