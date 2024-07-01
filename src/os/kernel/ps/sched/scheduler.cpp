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

scheduler &scheduler::current() noexcept { return schedulers_[hal::cpu_t::current_cpu_id()]; }

void scheduler::lock() noexcept { lock_depth_.fetch_add(1, std::memory_order_acq_rel); }
void scheduler::unlock() noexcept { lock_depth_.fetch_sub(1, std::memory_order_acq_rel); }

void scheduler::start_schedule() noexcept {
    threads_.insert_before(nullptr, &t_);
    while (1)
        ;
}

