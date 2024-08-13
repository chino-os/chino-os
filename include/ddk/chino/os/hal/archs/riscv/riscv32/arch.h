// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/os/kernel/kernel_types.h>
#include <chino/os/processapi.h>
#include <chino/units.h>
#include <chrono>
#include <cstddef>
#include <cstdint>

namespace chino::os::hal {
using arch_irq_state_t = uintptr_t;

enum class riscv32_irq_number {
    system_tick,
    syscall,
    host_console_stdin,
    host_serial_rx,
    host_serial_tx,
    __count,
};

using arch_irq_number_t = riscv32_irq_number;

class riscv32_arch {
  public:
    inline static constexpr size_t min_page_size = 4 * KiB;

    static void arch_startup() noexcept;

    static size_t current_cpu_id() noexcept {
        size_t hart_id;
        __asm volatile("csrr %0, mhartid" : "=r"(hart_id));
        return hart_id;
    }

    static std::chrono::nanoseconds current_cpu_time() noexcept;

    static void initialize_thread_stack(uintptr_t *&stack_top, kernel::ps::thread_main_thunk_t thunk,
                                        thread_start_t entrypoint, void *entry_arg) noexcept;

    static void yield_cpu() noexcept {}
    [[noreturn]] static void start_schedule(kernel::ps::thread &thread) noexcept;
    static void yield() noexcept;
    static void syscall(kernel::syscall_number number, void *arg) noexcept {
        (void)number;
        (void)arg;
        __asm volatile("ecall");
    }

    static bool in_irq_handler() noexcept;
    static void enable_irq() noexcept { __asm volatile("csrsi mstatus, 8"); }

    static arch_irq_state_t disable_irq() noexcept {
        uintptr_t state;
        __asm volatile("csrrci %0, mstatus, 8" : "=r"(state));
        return state;
    }

    static bool restore_irq(arch_irq_state_t state) noexcept {
        __asm volatile("csrw mstatus, %0" ::"r"(state));
        return state & 0x8;
    }

    static void send_irq(arch_irq_number_t irq_number);

    static void enable_system_tick(std::chrono::nanoseconds due_time) noexcept;
};

using arch_t = riscv32_arch;
} // namespace chino::os::hal
