// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "scheduler.h"
#include "../task/process.h"
#include <array>
#include <atomic>
#include <chino/os/hal/arch.h>
#include <chino/os/hal/chip.h>
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/kd.h>
#include <chino/os/kernel/ke.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::ps;
using namespace std::chrono_literals;

namespace {
constexpr auto system_tick_interval = std::chrono::milliseconds(1000);
constinit std::array<scheduler, hal::chip_t::cpus_count> schedulers_;
} // namespace

extern "C" {
std::array<std::atomic<thread *>, hal::chip_t::cpus_count> chino_current_threads;
}

void ps_switch_task() noexcept { scheduler::current().switch_task(); }

thread &ps::current_thread() noexcept {
    return *chino_current_threads[hal::arch_t::current_cpu_id()].load(std::memory_order_acquire);
}

process &ps::current_process() noexcept { return current_thread().process(); }

void ps::yield() noexcept {
    if (hal::arch_t::in_irq_handler()) {
        scheduler::current().switch_task();
    } else {
        current_irq_lock irq_lock;
        hal::arch_t::yield();
    }
}

scheduler &scheduler::current() noexcept { return schedulers_[hal::arch_t::current_cpu_id()]; }

void scheduler::unblock_threads(const void *wait_address, bool unblock_all) noexcept {
    current_irq_lock irq_lock;
    auto unblocked = scheduler::current().unblock_local_threads(wait_address, unblock_all);
    // IPI
    // if (unblock_all || !unblocked) {
    //    while (1)
    //        ;
    //}
    (void)unblocked;
}

void scheduler::initialize_phase0() noexcept {}

void scheduler::lock() noexcept { lock_depth_.fetch_add(1, std::memory_order_acq_rel); }
void scheduler::unlock() noexcept { lock_depth_.fetch_sub(1, std::memory_order_acq_rel); }

void scheduler::attach_thread(thread &thread) noexcept {
    thread.add_ref();

    current_irq_lock irq_lock;
    list_of(thread).push_back(&thread);
    update_max_ready_priority(thread.priority());
    thread.scheduler(this);
    thread.status(thread_status::ready);

    if (started_.load(std::memory_order_acquire)) {
        yield();
    }
}

void scheduler::detach_thread(thread &thread) noexcept {
    {
        current_irq_lock irq_lock;
        list_of(thread).remove(&thread);
        thread.scheduler(nullptr);
        if (thread.priority() == max_ready_priority_) {
            max_ready_priority_ = thread_priority::idle;
        }
    }
    thread.dec_ref();
}

void scheduler::switch_task() noexcept {
    current_irq_lock irq_lock;
    if (!lock_depth_.load(std::memory_order_acquire)) {
        auto &cnt_thread = current_thread();
        auto &next_thread = select_next_thread();
        if (&next_thread != &cnt_thread) {
            set_current_thread(next_thread);
        }
    }
}

void scheduler::start_schedule(thread &first_thread) noexcept {
    chino_current_threads[hal::arch_t::current_cpu_id()] = &first_thread;
    first_thread.status(thread_status::running);
    setup_next_system_tick();
    started_.store(1, std::memory_order_release);
    hal::arch_t::start_schedule(first_thread);
}

void scheduler::on_system_tick() noexcept {
    setup_next_system_tick();
    wakeup_delayed_threads();
    switch_task();
}

result<void> scheduler::block_current_thread(std::atomic<uint32_t> &wait_address, uint32_t old,
                                             std::optional<std::chrono::nanoseconds> timeout) noexcept {
    auto last_time = hal::arch_t::current_cpu_time();
    {
        current_irq_lock irq_lock;
        auto &cnt_thread = current_thread();
        if (wait_address.load(std::memory_order_acquire) == old) {
            cnt_thread.wait_address = &wait_address;
            list_of(cnt_thread).remove(&cnt_thread);

            if (!timeout) {
                // 1. Add to blocked list
                blocked_threads_.push_back(&cnt_thread);
                cnt_thread.status(thread_status::blocked);
            } else {
                // 2. Add to delayed list
                add_to_delay_list(cnt_thread, *timeout);
            }
        } else {
            return ok();
        }
    }

    // 3. Yield
    while (true) {
        yield();
        if (wait_address.load(std::memory_order_acquire) != old) {
            return ok();
        } else if (timeout && hal::arch_t::current_cpu_time() >= last_time + *timeout) {
            return err(error_code::timeout);
        }
    }
}

void scheduler::delay_current_thread(std::chrono::nanoseconds timeout) noexcept {}

scheduler::scheduler_list_t &scheduler::list_of(thread &thread) noexcept {
    switch (thread.status()) {
    case thread_status::ready:
    case thread_status::running:
        return ready_threads_[(size_t)thread.priority()];
    case thread_status::blocked:
        return blocked_threads_;
    case thread_status::delayed:
        return delayed_threads_;
    case thread_status::terminated:
        fail_fast("Terminated thread.");
    }
    CHINO_UNREACHABLE();
}

thread &scheduler::select_next_thread() noexcept {
    auto &cnt_thread = current_thread();
    auto cnt_priority = (uint32_t)cnt_thread.priority();
    auto search_priority = (uint32_t)max_ready_priority_;
    thread *next_thread = nullptr;

    if (search_priority > cnt_priority || cnt_thread.status() != thread_status::running) {
        if (cnt_thread.status() != thread_status::running) {
            cnt_priority = 0;
        }
        // 1. Higher thread ready or current thread blocked, search from max priority
        for (size_t i = search_priority; i >= cnt_priority; i--) {
            auto &threads = ready_threads_[i];
            if (!threads.empty()) {
                next_thread = threads.front();
                break;
            }
        }
    } else {
        // 2. Round robin the same priority
        auto &threads = list_of(cnt_thread);
        next_thread = threads.next(&cnt_thread);
        if (!next_thread)
            next_thread = threads.front();
    }

    kassert(next_thread);
    return *next_thread;
}

void scheduler::set_current_thread(thread &thread) noexcept {
    auto &cnt_thread = current_thread();
    if (&cnt_thread != &thread) {
        if (cnt_thread.status() == thread_status::running) {
            cnt_thread.status(thread_status::ready);
        }
        thread.status(thread_status::running);
        max_ready_priority_ = thread.priority();
        chino_current_threads[hal::arch_t::current_cpu_id()].store(&thread, std::memory_order_release);
    }
}

void scheduler::update_max_ready_priority(thread_priority priority) noexcept {
    if (max_ready_priority_ < priority) {
        max_ready_priority_ = priority;
    }
}

void scheduler::setup_next_system_tick() noexcept { hal::arch_t::enable_system_tick(system_tick_interval); }

bool scheduler::unblock_local_threads(const void *wait_address, bool unblock_all) noexcept {
    bool unblocked = false;
    auto thread = blocked_threads_.front();
    while (thread) {
        auto next = blocked_threads_.next(thread);
        if (thread->wait_address == wait_address) {
            thread->wait_address = nullptr;
            blocked_threads_.remove(thread);
            thread->status(thread_status::ready);
            list_of(*thread).push_back(thread);
            update_max_ready_priority(thread->priority());
            unblocked = true;
            if (!unblock_all)
                break;
        }
        thread = next;
    }
    if (unblocked)
        hal::arch_t::syscall(syscall_number::yield, nullptr);
    return unblocked;
}

void scheduler::add_to_delay_list(thread &thread, std::chrono::nanoseconds timeout) noexcept {
    auto wakeup_time = hal::arch_t::current_cpu_time() + timeout;
    auto pivot = delayed_threads_.front();
    if (pivot) {
        ps::thread *next;
        // 1. Find the last thread.wakeup_time <= cnt_thread.wakeup_time
        while ((next = delayed_threads_.next(pivot)) && next->wakeup_time <= wakeup_time) {
            pivot = next;
        }

        // 2. Add to wait list
        delayed_threads_.insert_after(pivot, &thread);
    } else {
        delayed_threads_.push_front(&thread);
    }
    thread.wakeup_time = wakeup_time;
    thread.status(thread_status::delayed);
}

void scheduler::wakeup_delayed_threads() noexcept {
    auto current_time = hal::arch_t::current_cpu_time();
    auto head = delayed_threads_.front();
    while (head && head->wakeup_time <= current_time) {
        auto wake = head;
        head = delayed_threads_.next(head);
        delayed_threads_.remove(wake);
        wake->status(thread_status::ready);
        list_of(*wake).push_back(wake);
        update_max_ready_priority(wake->priority());
    }
}
