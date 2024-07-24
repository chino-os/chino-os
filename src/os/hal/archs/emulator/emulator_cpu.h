// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <atomic>
#include <chino/os/hal/archs/emulator/arch.h>
#include <chino/os/hal/chip.h>

#ifdef WIN32
#include <Windows.h>
#endif

namespace chino::os::hal {
extern HINSTANCE hal_instance;

class emulator_cpu {
  public:
    static void register_message_queue();

    void run(size_t cpu_id, size_t memory_size);

    bool in_irq_handler() noexcept;
    arch_irq_state_t disable_irq();
    bool restore_irq(arch_irq_state_t irq_state);

    void enable_system_tick(std::chrono::milliseconds ticks);
    void syscall(kernel::syscall_number number, void *arg) noexcept;
    void send_irq(arch_irq_number_t irq_number);

  private:
    static DWORD WINAPI cpu_entry_thunk([[maybe_unused]] LPVOID pcpu);
    static LRESULT CALLBACK window_proc_thunk(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void process_irq(arch_irq_number_t irq_number, LPARAM lParam);

    void cpu_entry();
    void boot_cpu0();

    // System tick
    static VOID NTAPI on_system_tick_timer(_Inout_ PTP_CALLBACK_INSTANCE Instance, _Inout_opt_ PVOID Context,
                                           _Inout_ PTP_TIMER Timer);

  private:
    size_t cpu_id_;
    size_t memory_size_;
    HANDLE cpu_thread_;
    HWND event_window_;
    PTP_TIMER systick_timer_;
    arch_irq_number_t current_irq_;
    CONDITION_VARIABLE irq_state_cs_;
    CRITICAL_SECTION irq_lock_;
};
} // namespace chino::os::hal
