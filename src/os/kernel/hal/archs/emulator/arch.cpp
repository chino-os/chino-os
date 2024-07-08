// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
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
class emulator_cpu {
    inline static const wchar_t message_queue_class[] = L"Chino.Emulator.MQ";
    inline static constexpr UINT WM_ARCH_CALL = WM_USER + 1;
    inline static UINT_PTR systick_timer_id = 1;

    enum class arch_call_opcode { enable_systick };

    struct arch_call {
        arch_call(arch_call_opcode opcode) noexcept : opcode(opcode) {}
        arch_call_opcode opcode;
    };

    struct enable_systick_call : arch_call {
        enable_systick_call() noexcept : arch_call(arch_call_opcode::enable_systick) {}
        uint64_t ticks;
    };

  public:
    static void register_message_queue() {
        WNDCLASS wc = {};

        wc.lpfnWndProc = window_proc_thunk;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = message_queue_class;
        RegisterClass(&wc);
    }

    void run(size_t cpu_id, size_t memory_size) {
        cpu_id_ = cpu_id;
        memory_size_ = memory_size;

        // 1. Create event loop
        event_window_ = CreateWindowExW(0, message_queue_class, message_queue_class, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr,
                                        GetModuleHandleA(nullptr), this);
        cpu_thread_ = CreateThread(nullptr, 0, cpu_entry_thunk, (LPVOID)this, 0, nullptr);
        MSG msg = {};
        while (GetMessage(&msg, NULL, 0, 0) > 0) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void enable_systick(uint64_t ticks) {
        enable_systick_call params;
        params.ticks = ticks;
        PostMessage(event_window_, WM_ARCH_CALL, NULL, (LPARAM)&params);
    }

  private:
    static LRESULT CALLBACK window_proc_thunk(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (uMsg == WM_NCCREATE) {
            // Setup link
            auto create_struct = (CREATESTRUCTW *)lParam;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)create_struct->lpCreateParams);
        } else {
            auto this_ptr = (emulator_cpu *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if (this_ptr) {
                if (uMsg != WM_NCDESTROY) {
                    return this_ptr->window_proc(uMsg, wParam, lParam);
                } else {
                    SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
                }
            }
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    LRESULT window_proc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (uMsg == WM_ARCH_CALL) {
            auto *call = (arch_call *)lParam;
            switch (call->opcode) {
            case arch_call_opcode::enable_systick:
                do_enable_systick((enable_systick_call *)call);
                break;
            }
            return 1;
        } else if (uMsg == WM_TIMER) {
            if (wParam == systick_timer_id) {
                // systick
                KillTimer(event_window_, systick_timer_id);
                on_systick();
                return 0;
            }
        }
        return DefWindowProc(event_window_, uMsg, wParam, lParam);
    }

    static DWORD WINAPI cpu_entry_thunk([[maybe_unused]] LPVOID pcpu) {
        auto cpu = (emulator_cpu *)pcpu;
        cpu->cpu_entry();
        return 0;
    }

    static void on_systick() {
        ps_on_system_tick();
        wchar_t chars[] = L"Hello \n";
        WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), chars, std::size(chars), nullptr, nullptr);
    }

    void do_enable_systick(enable_systick_call *call) {
        auto p = SetTimer(event_window_, ++systick_timer_id, call->ticks / 10000 /* in ms */, nullptr);
        (void)p;
    }

    void dispatch_irqs() {}

    void cpu_entry() {
        if (cpu_id_ == 0) {
            boot_cpu0();
        }
    }

    void boot_cpu0() {
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

  private:
    size_t cpu_id_;
    size_t memory_size_;
    HANDLE cpu_thread_;
    HWND event_window_;
};

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

void emulator_arch::restore_context(emulator_thread_context &context) noexcept { emulator_restore_context(&context); }

arch_irq_state_t emulator_arch::disable_irq() noexcept { return 0; }

void emulator_arch::restore_irq(arch_irq_state_t state) noexcept { (void)state; }

void emulator_arch::enable_systick(uint64_t ticks) noexcept { emulator::current_cpu().enable_systick(ticks); }
