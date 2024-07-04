// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../thread.h"
#include <array>
#include <atomic>
#include <chino/os/kernel/threading.h>

namespace chino::os::kernel::ps {
class scheduler {
    using schedule_list_t = intrusive_list<thread, &thread::schedule_list_node>;

  public:
    static scheduler &current() noexcept;
    static thread &current_thread() noexcept;

    void initialize_phase0() noexcept;
    void lock() noexcept;
    void unlock() noexcept;

    void attach_thread(thread &thread) noexcept;

    [[noreturn]] void start_schedule(thread &init_thread) noexcept;

  private:
    thread *select_next_thread() noexcept;
    void set_current_thread(thread &thread) noexcept;

  private:
    std::atomic<uint32_t> lock_depth_;
    std::atomic_flag higher_thread_ready_;

    std::array<schedule_list_t, (size_t)thread_priority::max + 1> ready_threads_;
};
} // namespace chino::os::kernel::ps
