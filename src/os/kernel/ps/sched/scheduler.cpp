// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "scheduler.h"
#include "chino/os/kernel/hal/cpu/emulator/cpu.h"
#include <array>
#include <atomic>
#include <chino/os/kernel/hal/chip/emulator/chip.h>

using namespace chino::os::kernel;
using namespace chino::os::kernel::ps;

namespace {
constinit std::array<scheduler, hal::chip_t::cpus_count> schedulers_;
constinit thread t_;
} // namespace

extern "C" {
std::array<thread *, hal::chip_t::cpus_count> chino_current_threads;
}

scheduler &scheduler::current() noexcept { return schedulers_[hal::cpu_t::current_cpu_id()]; }
thread &scheduler::current_thread() noexcept { return *chino_current_threads[hal::cpu_t::current_cpu_id()]; }

void scheduler::lock() noexcept { lock_depth_.fetch_add(1, std::memory_order_acq_rel); }
void scheduler::unlock() noexcept { lock_depth_.fetch_sub(1, std::memory_order_acq_rel); }

thread *scheduler::select_next_thread() noexcept {
    // 1. Find from max priority
    auto &cnt_thread = current_thread();
    auto cnt_priority = (size_t)cnt_thread.priority();
    for (size_t i = ready_threads_.size() - 1; i > cnt_priority; i--) {
        auto &threads = ready_threads_[i];
        if (!threads.empty()) {
            return threads.front();
        }
    }

    auto next = schedule_list_t::next(&cnt_thread);
    return next ? next : ready_threads_[cnt_priority].front();
}

void scheduler::start_schedule() noexcept {
    while (1)
        ;
}
