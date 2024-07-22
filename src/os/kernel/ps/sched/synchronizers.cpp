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

void waitable_object::blocking_wait(std::optional<std::chrono::milliseconds> timeout,
                                    hal::arch_irq_state_t irq_state) noexcept {
    auto &cnt_thread = current_thread();
    auto &list = waiting_list(waiting_threads_);
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

    // 3. Block this thread
    scheduler::current().block_current_thread(*this, timeout, syncroot_, irq_state);
}

void waitable_object::notify_one() noexcept {
    auto &list = waiting_list(waiting_threads_);
    auto irq_state = syncroot_.lock();
    auto pivot = list.front();
    if (pivot) {
        list.remove(pivot);
        scheduler::unblock_thread(*pivot, syncroot_, irq_state);
    } else {
        syncroot_.unlock(irq_state);
    }
}

void waitable_object::notify_all() noexcept {
    auto &list = waiting_list(waiting_threads_);
    while (true) {
        auto irq_state = syncroot_.lock();
        auto pivot = list.front();
        if (pivot) {
            list.remove(pivot);
            scheduler::unblock_thread(*pivot, syncroot_, irq_state);
        } else {
            syncroot_.unlock(irq_state);
            break;
        }
    }
}

result<void> mutex::try_wait() noexcept {
    uint32_t held = 0;
    return held_.compare_exchange_strong(held, 1, std::memory_order_acq_rel) ? ok() : err(error_code::unavailable);
}

result<void> mutex::wait(std::optional<std::chrono::milliseconds> timeout) noexcept {
    if (try_wait().is_ok()) {
        // 1. Fast path
        return ok();
    } else {
        // 2. Slow path
        auto irq_state = syncroot().lock();
        if (try_wait().is_ok()) {
            syncroot().unlock(irq_state);
            return ok();
        } else {
            blocking_wait(timeout, irq_state);
            return try_wait().is_ok() ? ok() : err(error_code::timeout);
        }
    }
}

void mutex::release() noexcept {
    held_.store(0, std::memory_order_release);
    if (!held_.load(std::memory_order_acquire)) {
        notify_one();
    }
}

result<void> event::try_wait() noexcept {
    return signal_.load(std::memory_order_acquire) ? ok() : err(error_code::unavailable);
}

result<void> event::wait(std::optional<std::chrono::milliseconds> timeout) noexcept {
    if (try_wait().is_ok()) {
        // 1. Fast path
        return ok();
    } else {
        // 2. Slow path
        auto irq_state = syncroot().lock();
        if (try_wait().is_ok()) {
            syncroot().unlock(irq_state);
            return ok();
        } else {
            blocking_wait(timeout, irq_state);
            return try_wait().is_ok() ? ok() : err(error_code::timeout);
        }
    }
}

void event::notify_all() noexcept {
    signal_.store(1, std::memory_order_release);
    waitable_object::notify_all();
}

void event::reset() noexcept { signal_.store(0, std::memory_order_release); }

result<void> condition_variable::wait(mutex &mutex, std::optional<std::chrono::milliseconds> timeout) noexcept {
    if (event_.try_wait().is_ok()) {
        // 1. Fast path
        return ok();
    } else {
        // 2. Slow path
        mutex.release();
        auto result = event_.wait(timeout);
        try_(mutex.wait());
        return result;
    }
}

result<void> condition_variable::notify() noexcept { return ok(); }
