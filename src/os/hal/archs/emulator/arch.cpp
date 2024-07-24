// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "emulator.h"
#include "emulator_cpu.h"
#include <chino/compiler.h>
#include <chino/os/hal/archs/emulator/arch.h>
#include <chino/os/hal/chip.h>
#include <chino/os/kernel/ke.h>
#include <memory>

using namespace chino;
using namespace chino::os::hal;
using namespace chino::os::kernel;

extern "C" {
void emulator_restore_irq(arch_irq_state_t irq_state) noexcept { emulator_arch::restore_irq(irq_state); }

[[noreturn]] extern void emulator_start_schedule() noexcept;
extern void emulator_yield() noexcept;
}

void emulator_arch::arch_startup(size_t memory_size) { emulator::run(memory_size); }

std::chrono::milliseconds emulator_arch::current_cpu_time() noexcept {
    return std::chrono::milliseconds(GetTickCount64());
}

void emulator_arch::initialize_thread_stack(uintptr_t *&stack_top, kernel::ps::thread_main_thunk_t thunk,
                                            thread_start_t entrypoint, void *entry_arg) noexcept {
    *--stack_top = 0;                     // Avoid unwind
    *--stack_top = (uintptr_t)thunk;      // return address: thunk
    stack_top -= 11;                      // r15-r8, rdi, rsi, rbp = 0
    *--stack_top = (uintptr_t)entry_arg;  // rdx = entry_arg
    *--stack_top = (uintptr_t)entrypoint; // rcx = entrypoint
    stack_top -= 2;                       // rbx, rax = 0
    *--stack_top = 1;                     // irq = 1
}

void emulator_arch::yield_cpu() noexcept { YieldProcessor(); }

void emulator_arch::start_schedule(ps::thread &thread) noexcept {
    __asm {
        jmp emulator_start_schedule
    }
}

void emulator_arch::yield() noexcept {
    __asm {
        jmp emulator_yield
    }
}

void emulator_arch::syscall(kernel::syscall_number number, void *arg) noexcept {
    emulator::current_cpu().syscall(number, arg);
}

bool emulator_arch::in_irq_handler() noexcept { return emulator::current_cpu().in_irq_handler(); }

void emulator_arch::enable_irq() noexcept { restore_irq(1); }

arch_irq_state_t emulator_arch::disable_irq() noexcept { return emulator::current_cpu().disable_irq(); }

bool emulator_arch::restore_irq(arch_irq_state_t state) noexcept { return emulator::current_cpu().restore_irq(state); }

void emulator_arch::send_irq(arch_irq_number_t irq_number) { emulator::current_cpu().send_irq(irq_number); }

void emulator_arch::enable_system_tick(std::chrono::milliseconds due_time) noexcept {
    emulator::current_cpu().enable_system_tick(due_time);
}
