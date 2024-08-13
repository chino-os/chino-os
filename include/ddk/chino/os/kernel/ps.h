// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../object.h"
#include <atomic>
#include <chino/intrusive_list.h>
#include <chino/os/hal/arch.h>
#include <chino/os/processapi.h>
#include <mutex>
#include <span>

namespace chino::os::kernel::ps {
class process;
class thread;

struct thread_create_options {
    ps::process *process;
    thread_priority priority = thread_priority::normal;
    bool not_owned_stack = false;
    std::span<uintptr_t> stack;
    thread_start_t entry_point;
    void *entry_arg = nullptr;
};

struct current_schedule_lock {
    CHINO_NONCOPYABLE(current_schedule_lock);

    current_schedule_lock() noexcept;
    ~current_schedule_lock();
};

class current_irq_lock {
  public:
    CHINO_NONCOPYABLE(current_irq_lock);

    current_irq_lock() noexcept : state_(hal::arch_t::disable_irq()) {}
    ~current_irq_lock() { hal::arch_t::restore_irq(state_); }

  private:
    hal::arch_irq_state_t state_;
};

class irq_spin_lock {
  public:
    hal::arch_irq_state_t lock() noexcept {
        auto irq_state = hal::arch_t::disable_irq();
        uint32_t expected = 0;
        while (!held_.compare_exchange_strong(expected, 1, std::memory_order_acq_rel)) {
            expected = 0;
            hal::arch_t::yield_cpu();
        }
        return irq_state;
    }

    void unlock(hal::arch_irq_state_t irq_state) noexcept {
        held_.store(0, std::memory_order_release);
        hal::arch_t::restore_irq(irq_state);
    }

  private:
    std::atomic<uint32_t> held_;
};

result<void> create_process(std::string_view filepath, lazy_construct<thread> &thread,
                            thread_create_options &options) noexcept;
result<void> create_process(std::string_view filepath) noexcept;

thread &current_thread() noexcept;
process &current_process() noexcept;
void yield() noexcept;
} // namespace chino::os::kernel::ps

namespace std {
template <class T> class unique_lock;

template <> class unique_lock<chino::os::kernel::ps::irq_spin_lock> {
  public:
    CHINO_NONCOPYABLE(unique_lock);

    unique_lock(chino::os::kernel::ps::irq_spin_lock &spin_lock) noexcept
        : lock_(spin_lock), irq_state_(spin_lock.lock()) {}
    ~unique_lock() { lock_.unlock(irq_state_); }

  private:
    chino::os::kernel::ps::irq_spin_lock &lock_;
    chino::os::hal::arch_irq_state_t irq_state_;
};
} // namespace std

extern "C" {
void ps_switch_task() noexcept;
}
