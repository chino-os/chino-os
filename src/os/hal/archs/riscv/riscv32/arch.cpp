// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "../../../../kernel/ps/task/thread.h"
#include <chino/compiler.h>
#include <chino/os/hal/archs/riscv/riscv32/arch.h>
#include <chino/os/hal/chip.h>
#include <chino/os/kernel/kd.h>
#include <chino/os/kernel/ke.h>
#include <chrono>

using namespace chino;
using namespace chino::os::hal;
using namespace chino::os::kernel;

void riscv32_arch::arch_startup() noexcept {}

std::chrono::nanoseconds riscv32_arch::current_cpu_time() noexcept { return std::chrono::nanoseconds(0); }

void riscv32_arch::initialize_thread_stack(uintptr_t *&stack_top, kernel::ps::thread_main_thunk_t thunk,
                                           thread_start_t entrypoint, void *entry_arg) noexcept {
    *--stack_top = 0;                     // Avoid unwind
    *--stack_top = (uintptr_t)thunk;      // return address: thunk
    stack_top -= 11;                      // r15-r8, rdi, rsi, rbp = 0
    *--stack_top = (uintptr_t)entry_arg;  // rdx = entry_arg
    *--stack_top = (uintptr_t)entrypoint; // rcx = entrypoint
    stack_top -= 2;                       // rbx, rax = 0
}

void riscv32_arch::start_schedule(ps::thread &thread) noexcept {
    (void)thread;
    __asm volatile("lw t0,     0x4(a0)\n");
    CHINO_UNREACHABLE();
}

void riscv32_arch::yield() noexcept { __asm volatile("lw t0,     0x4(a0)\n"); }

bool riscv32_arch::in_irq_handler() noexcept { return false; }

void riscv32_arch::enable_system_tick(std::chrono::nanoseconds due_time) noexcept { (void)due_time; }
