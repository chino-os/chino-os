// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "emulator_cpu.h"
#include <chino/compiler.h>
#include <chino/os/kernel/hal/archs/emulator/arch.h>
#include <chino/os/kernel/hal/chip.h>
#include <chino/os/kernel/ke.h>
#include <memory>

#ifdef WIN32
#include <Windows.h>
#endif

using namespace chino;
using namespace chino::os::kernel;
using namespace chino::os::kernel::hal;

extern "C" {
[[noreturn]] extern void emulator_restore_context(emulator_thread_context *context) noexcept;
}

namespace {
class emulator {
  public:
    static void run(size_t memory_size) {
        memory_size_ = memory_size;
        emulator_cpu::register_message_queue();
        create_cpu_threads();
        WaitForMultipleObjects((DWORD)cpu_threads_.size(), cpu_threads_.data(), TRUE, INFINITE);
    }

    static emulator_cpu &cpu(size_t id) { return cpus_[id]; }

    static emulator_cpu &current_cpu() { return cpu(emulator_arch::current_cpu_id()); }

  private:
    static void create_cpu_threads() {
        for (size_t i = 0; i < cpu_threads_.size(); i++) {
            cpu_threads_[i] = CreateThread(nullptr, 0, cpu_entry, (LPVOID)i, 0, nullptr);
        }
    }

    static DWORD WINAPI cpu_entry([[maybe_unused]] LPVOID pcpu_id) {
        auto cpu_id = (size_t)pcpu_id;
        cpu(cpu_id).run(cpu_id, memory_size_);
        return 0;
    }

    static void restore_context(emulator_thread_context &context) noexcept { ResumeThread(cpu_threads_[0]); }

  private:
    inline static size_t memory_size_;
    inline static std::array<HANDLE, chip_t::cpus_count> cpu_threads_;
    inline static std::array<emulator_cpu, chip_t::cpus_count> cpus_;
};
} // namespace

void emulator_arch::arch_startup(size_t memory_size) { emulator::run(memory_size); }

emulator_thread_context emulator_arch::initialize_thread_context(std::span<std::byte> stack,
                                                                 ps::thread_main_thunk_t thread_thunk, void *thread,
                                                                 thread_start_t entry_point, void *entry_arg) noexcept {
    auto rsp = reinterpret_cast<uintptr_t *>(stack.data() + stack.size_bytes());
    *--rsp = 0; // Avoid unwind
    return emulator_thread_context{.rcx = (uintptr_t)thread,
                                   .rdx = (uintptr_t)entry_point,
                                   .rsp = (uintptr_t)rsp,
                                   .r8 = (uintptr_t)entry_arg,
                                   .rip = (uintptr_t)thread_thunk};
}

std::chrono::milliseconds emulator_arch::current_cpu_time() noexcept {
    return std::chrono::milliseconds(GetTickCount64());
}

void emulator_arch::restore_context(emulator_thread_context &context) noexcept { emulator_restore_context(&context); }

arch_irq_state_t emulator_arch::disable_irq() noexcept { return emulator::current_cpu().disable_irq(); }

void emulator_arch::restore_irq(arch_irq_state_t state) noexcept { (void)state; }

void emulator_arch::enable_system_tick(std::chrono::milliseconds due_time) noexcept {
    emulator::current_cpu().enable_system_tick(due_time);
}
