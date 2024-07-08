// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "scheduler.h"

using namespace chino;
using namespace chino::os::kernel;
using namespace chino::os::kernel::ps;

namespace {
using waiting_list_t = intrusive_list<thread, &thread::waiting_list_node>;

waiting_list_t &waiting_list(chino::detail::intrusive_list_storage &list) noexcept {
    return reinterpret_cast<waiting_list_t &>(list);
}
} // namespace

current_schedule_lock::current_schedule_lock() noexcept { scheduler::current().lock(); }
current_schedule_lock::~current_schedule_lock() { scheduler::current().unlock(); }

void waitable_object::blocking_wait(std::optional<std::chrono::milliseconds> timeout) noexcept {
    auto &cnt_thread = scheduler::current_thread();

    auto &list = waiting_list(waiting_threads_);
    {
        std::unique_lock locker(waiting_threads_lock_);
        auto pivot = list.front();
        if (pivot) {
            thread *next;
            // 1. Find the last thread.priority >= cnt_thread.priority
            while ((next = list.next(pivot)) && next->priority() >= cnt_thread.priority()) {
                pivot = next;
            }

            // 2. Add to wait list
            list.insert_after(pivot, &cnt_thread);
        } else {
            list.push_front(&cnt_thread);
        }
    }

    // 3. Block this thread
    scheduler::current().block_current_thread(*this, timeout);
}

void waitable_object::notify_one() noexcept {
    auto &list = waiting_list(waiting_threads_);
    auto pivot = list.front();
    if (pivot) {
        list.remove(pivot);
        scheduler::current().unblock_thread(*pivot);
    }
}

result<void> mutex::wait(std::optional<std::chrono::milliseconds> timeout) noexcept {
    uint32_t held = 0;
    if (held_.compare_exchange_strong(held, 1, std::memory_order_acq_rel)) {
        return ok();
    } else {
        held = 0;
        if (held_.compare_exchange_strong(held, 1, std::memory_order_acq_rel)) {
            return ok();
        } else {
            blocking_wait(timeout);
            return held_.compare_exchange_strong(held, 1, std::memory_order_acq_rel) ? ok() : err(error_code::timeout);
        }
    }
}

void mutex::release() noexcept {
    held_.store(0, std::memory_order_release);
    if (!held_.load(std::memory_order_acquire)) {
        notify_one();
    }
}
