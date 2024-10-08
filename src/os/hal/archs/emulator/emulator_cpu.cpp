// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "emulator_cpu.h"
#include "../../../kernel/ps/task/thread.h"
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/kd.h>
#include <chino/os/kernel/ke.h>

using namespace chino;
using namespace chino::os::hal;
using namespace chino::os::kernel;

namespace {
inline static const wchar_t message_queue_class[] = L"Chino.Emulator.MQ";
inline static constexpr UINT WM_ARCH_IRQ = WM_USER + 1;
} // namespace

extern "C" {
std::atomic<arch_irq_state_t> emulator_irq_state;
std::atomic<uint32_t> emulator_in_irq_handler;

struct syscall_payload {
    syscall_number number;
    void *arg;
};

[[noreturn]] extern void emulator_dispatch_irq(arch_irq_number_t irq_number) noexcept;
extern void emulator_dispatch_irq_end();

static syscall_payload syscall_payload_;
}

void emulator_cpu::register_message_queue() {
    WNDCLASS wc = {};

    wc.lpfnWndProc = window_proc_thunk;
    wc.hInstance = hal_instance;
    wc.lpszClassName = message_queue_class;
    RegisterClass(&wc);
}

void emulator_cpu::run(size_t cpu_id, size_t memory_size) {
    cpu_id_ = cpu_id;
    memory_size_ = memory_size;

    // Create IRQ lock
    emulator_irq_state.store(0, std::memory_order_relaxed); // Disable IRQ on startup
    InitializeCriticalSection(&irq_lock_);
    InitializeConditionVariable(&irq_state_cs_);

    // Create Systick timer
    systick_timer_ = CreateThreadpoolTimer(on_system_tick_timer, this, nullptr);

    // Create event loop window
    event_window_ = CreateWindowExW(0, message_queue_class, message_queue_class, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                                    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hal_instance, this);
    SetThreadDescription(GetCurrentThread(), L"Chino Emulator APIC");

    cpu_thread_ = CreateThread(nullptr, 0, cpu_entry_thunk, (LPVOID)this, 0, nullptr);
    SetThreadDescription(cpu_thread_, L"Chino Emulator CPU");

    // Begin event loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void emulator_cpu::start_schedule(ps::thread &thread) noexcept {
    current_thread_ = (HANDLE)thread.emulator_handle;
    ResumeThread(current_thread_);
    SuspendThread(GetCurrentThread());
    CHINO_UNREACHABLE();
}

void emulator_cpu::yield() noexcept {
    auto last_thread = current_thread_.load(std::memory_order_acquire);
    ps::current_thread().emulator_irq_state = emulator_irq_state.load();
    ps_switch_task();
    auto new_thread = (HANDLE)ps::current_thread().emulator_handle;
    if (last_thread != new_thread) {
        current_thread_.store(new_thread, std::memory_order_release);
        restore_irq(ps::current_thread().emulator_irq_state);
        kassert(ResumeThread(new_thread) != -1);
        kassert(SuspendThread(last_thread) != -1);
    }
}

bool emulator_cpu::in_irq_handler() noexcept { return emulator_in_irq_handler.load(std::memory_order_acquire); }

arch_irq_state_t emulator_cpu::disable_irq() {
    arch_irq_state_t old_state = 1;
    EnterCriticalSection(&irq_lock_);
    emulator_irq_state.compare_exchange_weak(old_state, 0, std::memory_order_relaxed);
    LeaveCriticalSection(&irq_lock_);
    return old_state;
}

bool emulator_cpu::restore_irq(arch_irq_state_t irq_state) {
    EnterCriticalSection(&irq_lock_);
    emulator_irq_state.store(irq_state, std::memory_order_relaxed);
    LeaveCriticalSection(&irq_lock_);
    if (irq_state) {
        WakeConditionVariable(&irq_state_cs_);
    }
    return irq_state;
}

LRESULT CALLBACK emulator_cpu::window_proc_thunk(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_NCCREATE) {
        // Setup link
        auto create_struct = (CREATESTRUCTW *)lParam;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)create_struct->lpCreateParams);
    } else {
        auto this_ptr = (emulator_cpu *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (this_ptr) {
            if (uMsg != WM_NCDESTROY) {
                return this_ptr->window_proc(hwnd, uMsg, wParam, lParam);
            } else {
                SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
            }
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT emulator_cpu::window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_ARCH_IRQ) {
        process_irq((arch_irq_number_t)wParam, lParam);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void emulator_cpu::send_irq(arch_irq_number_t irq_number) {
    PostMessage(event_window_, WM_ARCH_IRQ, (WPARAM)irq_number, NULL);
}

void emulator_cpu::syscall(syscall_number number, void *arg) noexcept {
    syscall_payload_ = {.number = number, .arg = arg};
    PostMessage(event_window_, WM_ARCH_IRQ, (WPARAM)arch_irq_number_t::syscall, (LPARAM)&syscall_payload_);
}

void emulator_cpu::process_irq(arch_irq_number_t irq_number, LPARAM lParam) {
    EnterCriticalSection(&irq_lock_);
    while (!emulator_irq_state.load()) {
        SleepConditionVariableCS(&irq_state_cs_, &irq_lock_, INFINITE);
    }

    auto cnt_thread = current_thread_.load(std::memory_order_acquire);
    SuspendThread(cnt_thread);
    LeaveCriticalSection(&irq_lock_);

    emulator_in_irq_handler.store(1, std::memory_order_release);
    ps::current_thread().emulator_irq_state = emulator_irq_state.load();
    auto last_thread = current_thread_.load(std::memory_order_acquire);
    if (irq_number == arch_irq_number_t::syscall) {
        auto *payload = reinterpret_cast<syscall_payload *>(lParam);
        io_handle_irq(irq_number, payload->number, payload->arg);
    } else {
        io_handle_irq(irq_number, (syscall_number)0, nullptr);
    }
    auto new_thread = (HANDLE)ps::current_thread().emulator_handle;
    if (last_thread != new_thread) {
        current_thread_.store(new_thread, std::memory_order_release);
    }
    emulator_irq_state.store(ps::current_thread().emulator_irq_state);
    emulator_in_irq_handler.store(0, std::memory_order_release);
    kassert(ResumeThread(new_thread) != -1);
}

DWORD WINAPI emulator_cpu::cpu_entry_thunk([[maybe_unused]] LPVOID pcpu) {
    auto cpu = (emulator_cpu *)pcpu;
    cpu->cpu_entry();
    return 0;
}

void emulator_cpu::cpu_entry() {
    if (cpu_id_ == 0) {
        boot_cpu0();
    }
}

void emulator_cpu::boot_cpu0() {
    // 1. Parpare boot options
    // 1.1. Create memory
    auto free_pages = memory_size_ / arch_t::min_page_size;
    size_t free_memory_size = arch_t::min_page_size * free_pages;
    auto free_memory = VirtualAlloc(nullptr, free_memory_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!free_memory) {
        fail_fast("Unable to allocate main memory.");
    }

    boot_memory_desc memory_descs[] = {
        {
            .kind = boot_memory_kind::free,
            .physical_address = (uintptr_t)free_memory,
            .virtual_address = (uintptr_t)free_memory,
            .size_bytes = free_memory_size,
        },
    };

    // 1.2 Create boot options
    boot_options options{.memory_descs = std::span(memory_descs)};

    // 2. Startup
    ke_startup(options);
}
