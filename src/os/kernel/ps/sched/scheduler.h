// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../task/thread.h"
#include <array>
#include <atomic>
#include <chino/os/kernel/ps.h>

namespace chino::os::kernel::ps {
class scheduler {
    using scheduler_list_t = intrusive_list<thread, &thread::scheduler_list_node>;

  public:
    static scheduler &current() noexcept;
    static thread &current_thread() noexcept;

    constexpr scheduler() noexcept : max_ready_priority_(thread_priority::idle), current_time_(0) {}

    void initialize_phase0() noexcept;
    void lock() noexcept;
    void unlock() noexcept;

    void attach_thread(thread &thread) noexcept;
    void detach_thread(thread &thread) noexcept;
    void yield() noexcept;

    void block_current_thread(waitable_object &waiting_object,
                              std::optional<std::chrono::milliseconds> timeout) noexcept;
    void delay_current_thread(std::chrono::milliseconds timeout) noexcept;

    [[noreturn]] void start_schedule() noexcept;
    void on_system_tick(std::chrono::milliseconds duration) noexcept;

  private:
    scheduler_list_t &list_of(thread &thread) noexcept;
    thread &select_next_thread() noexcept;
    void set_current_thread(thread &thread) noexcept;

    void setup_idle_thread() noexcept;
    void update_max_ready_priority(thread_priority priority) noexcept;
    void wakeup_delayed_threads() noexcept;
    void setup_next_system_tick() noexcept;

  private:
    std::atomic<uint32_t> lock_depth_;
    thread_priority max_ready_priority_;

    std::array<scheduler_list_t, (size_t)thread_priority::max + 1> ready_threads_;
    scheduler_list_t blocked_threads_;
    scheduler_list_t delayed_threads_;

    std::chrono::milliseconds current_time_;
};
} // namespace chino::os::kernel::ps
