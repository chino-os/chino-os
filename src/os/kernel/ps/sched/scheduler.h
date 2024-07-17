// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../task/thread.h"
#include <array>
#include <atomic>
#include <chino/os/kernel/ps.h>

namespace chino::os::kernel::ps {
class process;

class scheduler {
    using scheduler_list_t = intrusive_list<thread, &thread::scheduler_list_node>;

  public:
    static scheduler &current() noexcept;
    static thread &current_thread() noexcept;
    static process &current_process() noexcept { return current_thread().process(); }

    static void unblock_thread(thread &thread, irq_spin_lock &lock, hal::arch_irq_state_t irq_state) noexcept;

    constexpr scheduler() noexcept : max_ready_priority_(thread_priority::idle), current_time_(0) {}

    void initialize_phase0() noexcept;
    void lock() noexcept;
    void unlock() noexcept;

    void attach_thread(thread &thread) noexcept;
    void detach_thread(thread &thread) noexcept;
    void yield() noexcept;
    void yield_if_needed() noexcept;

    void block_current_thread(waitable_object &waiting_object, std::optional<std::chrono::milliseconds> timeout,
                              irq_spin_lock &lock, hal::arch_irq_state_t irq_state) noexcept;
    void delay_current_thread(std::chrono::milliseconds timeout) noexcept;

    [[noreturn]] void start_schedule(thread &first_thread) noexcept;
    void on_system_tick() noexcept;

  private:
    scheduler_list_t &list_of(thread &thread) noexcept;
    thread &select_next_thread() noexcept;
    void set_current_thread(thread &thread) noexcept;

    void update_max_ready_priority(thread_priority priority) noexcept;
    void setup_next_system_tick() noexcept;

    void unblock_local_thread(thread &thread, irq_spin_lock &lock, hal::arch_irq_state_t irq_state) noexcept;
    void add_to_delay_list(thread &thread, std::chrono::milliseconds timeout) noexcept;
    void wakeup_delayed_threads() noexcept;

  private:
    std::atomic<uint32_t> lock_depth_;
    thread_priority max_ready_priority_;

    std::array<scheduler_list_t, (size_t)thread_priority::max + 1> ready_threads_;
    scheduler_list_t blocked_threads_;
    scheduler_list_t delayed_threads_;

    std::chrono::milliseconds current_time_;
};
} // namespace chino::os::kernel::ps
