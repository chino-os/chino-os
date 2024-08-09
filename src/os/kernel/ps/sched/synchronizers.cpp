// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "../../ke/ke_services.h"
#include "scheduler.h"
#include <chino/os/kernel/kd.h>

using namespace chino;
using namespace chino::os::kernel;
using namespace chino::os::kernel::ps;

current_schedule_lock::current_schedule_lock() noexcept { scheduler::current().lock(); }
current_schedule_lock::~current_schedule_lock() { scheduler::current().unlock(); }

result<void> kernel_ke_service_mt::atomic_wait(std::atomic<uint32_t> &atomic, uint32_t old,
                                               std::optional<std::chrono::milliseconds> timeout) noexcept {
    if (atomic.load(std::memory_order_acquire) != old) {
        // 1. Fast path
        return ok();
    } else {
        // 2. Slow path
        return ps::scheduler::current().block_current_thread(atomic, old, timeout);
    }
}

void kernel_ke_service_mt::atomic_notify_one(std::atomic<uint32_t> &atomic) noexcept {
    ps::scheduler::unblock_threads(&atomic, false);
}

void kernel_ke_service_mt::atomic_notify_all(std::atomic<uint32_t> &atomic) noexcept {
    ps::scheduler::unblock_threads(&atomic, true);
}