// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/os/kernel/hal/archs/emulator/arch.h>
#include <chino/os/kernel/hal/chip.h>

#ifdef WIN32
#include <Windows.h>
#endif

namespace chino::os::kernel::hal {
extern HINSTANCE hal_instance;

enum class arch_call_opcode { enable_systick };

struct arch_call {
    arch_call(arch_call_opcode opcode) noexcept : opcode(opcode) {}
    arch_call_opcode opcode;
};

struct enable_systick_call : arch_call {
    enable_systick_call() noexcept : arch_call(arch_call_opcode::enable_systick) {}
    uint64_t ticks_in_ms;
};

class emulator_cpu {
    inline static UINT_PTR systick_timer_id = 1;

  public:
    static void register_message_queue();

    void run(size_t cpu_id, size_t memory_size);

    void enable_system_tick(std::chrono::milliseconds ticks);

  private:
    static DWORD WINAPI cpu_entry_thunk([[maybe_unused]] LPVOID pcpu);
    static DWORD WINAPI apic_entry_thunk([[maybe_unused]] LPVOID pcpu);
    static LRESULT CALLBACK window_proc_thunk(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT window_proc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void send_arch_call(arch_call &call);
    void send_irq(arch_irq_number_t irq_number);

    void cpu_entry();
    void apic_entry();
    void boot_cpu0();

    // System tick
    void on_system_tick();
    void do_enable_systick(enable_systick_call *call);

  private:
    size_t cpu_id_;
    size_t memory_size_;
    HANDLE cpu_thread_;
    HANDLE apic_thread_;
    HWND event_window_;
    HWND apic_window_;
    arch_irq_number_t current_irq_;
};
} // namespace chino::os::kernel::hal
