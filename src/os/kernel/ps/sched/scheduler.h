// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../thread.h"
#include <atomic>
#include <chino/os/kernel/threading.h>

namespace chino::os::kernel::ps {
class scheduler {
  public:
    static scheduler &current() noexcept;

    void initialize_phase0() noexcept;
    void lock() noexcept;
    void unlock() noexcept;

    [[noreturn]] void start_schedule() noexcept;

  private:
    std::atomic<uint32_t> lock_depth_;

    intrusive_list<thread, &thread::schedule_list_node> threads_;
};
} // namespace chino::os::kernel::ps
