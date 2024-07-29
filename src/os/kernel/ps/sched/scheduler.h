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

    static void unblock_threads(const void *wait_address, bool unblock_all) noexcept;

    constexpr scheduler() noexcept : max_ready_priority_(thread_priority::idle), current_time_(0) {}

    void initialize_phase0() noexcept;
    void lock() noexcept;
    void unlock() noexcept;

    void switch_task() noexcept;

    void attach_thread(thread &thread) noexcept;
    void detach_thread(thread &thread) noexcept;

    result<void> block_current_thread(std::atomic<uint32_t> &wait_address, uint32_t old,
                                      std::optional<std::chrono::nanoseconds> timeout) noexcept;
    void delay_current_thread(std::chrono::nanoseconds timeout) noexcept;

    [[noreturn]] void start_schedule(thread &first_thread) noexcept;
    void on_system_tick() noexcept;

  private:
    scheduler_list_t &list_of(thread &thread) noexcept;
    thread &select_next_thread() noexcept;
    void set_current_thread(thread &thread) noexcept;

    void update_max_ready_priority(thread_priority priority) noexcept;
    void setup_next_system_tick() noexcept;

    bool unblock_local_threads(void *wait_address, bool unblock_all) noexcept;
    void add_to_delay_list(thread &thread, std::chrono::nanoseconds timeout) noexcept;
    void wakeup_delayed_threads() noexcept;

  private:
    std::atomic<uint32_t> started_;
    std::atomic<uint32_t> lock_depth_;
    thread_priority max_ready_priority_;

    std::array<scheduler_list_t, (size_t)thread_priority::max + 1> ready_threads_;
    scheduler_list_t blocked_threads_;
    scheduler_list_t delayed_threads_;

    std::chrono::nanoseconds current_time_;
};
} // namespace chino::os::kernel::ps
