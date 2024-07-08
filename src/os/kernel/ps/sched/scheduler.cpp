// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "scheduler.h"
#include "../task/process.h"
#include "chino/os/kernel/hal/arch.h"
#include "chino/os/kernel/hal/chip.h"
#include <array>
#include <atomic>
#include <chino/os/kernel/ke.h>

using namespace chino::os::kernel;
using namespace chino::os::kernel::ps;
using namespace std::chrono_literals;

namespace {
constinit std::array<scheduler, hal::chip_t::cpus_count> schedulers_;

alignas(hal::cacheline_size) std::array<std::byte, sizeof(uintptr_t) * 16> idle_stack_;
constinit static_object<ps::thread> idle_thread_;
} // namespace

extern "C" {
std::array<std::atomic<thread *>, hal::chip_t::cpus_count> chino_current_threads;
}

[[noreturn]] static int ps_idle_thread_main(void *arg) noexcept {
    while (true) {
    }
}

scheduler &scheduler::current() noexcept { return schedulers_[hal::arch_t::current_cpu_id()]; }
thread &scheduler::current_thread() noexcept {
    return *chino_current_threads[hal::arch_t::current_cpu_id()].load(std::memory_order_acquire);
}

void scheduler::initialize_phase0() noexcept { setup_idle_thread(); }

void scheduler::lock() noexcept { lock_depth_.fetch_add(1, std::memory_order_acq_rel); }
void scheduler::unlock() noexcept { lock_depth_.fetch_sub(1, std::memory_order_acq_rel); }

void scheduler::attach_thread(thread &thread) noexcept {
    thread.add_ref();

    current_irq_lock irq_lock;
    list_of(thread).push_back(&thread);
    update_max_ready_priority(thread.priority());
}

void scheduler::detach_thread(thread &thread) noexcept {
    {
        current_irq_lock irq_lock;
        list_of(thread).remove(&thread);
    }
    thread.dec_ref();
}

void scheduler::yield() noexcept {
    auto &next_thread = select_next_thread();
    set_current_thread(next_thread);
    hal::arch_t::restore_context(next_thread.context.arch);
}

void scheduler::start_schedule() noexcept {
    auto &next_thread = select_next_thread();
    set_current_thread(next_thread);
    setup_next_system_tick();
    hal::arch_t::restore_context(next_thread.context.arch);
}

void scheduler::on_system_tick() noexcept {
    current_time_ = hal::arch_t::current_cpu_time();
    setup_next_system_tick();
}

void scheduler::block_current_thread(waitable_object &waiting_object,
                                     std::optional<std::chrono::milliseconds> timeout) noexcept {
    auto &cnt_thread = current_thread();
    list_of(cnt_thread).remove(&cnt_thread);
    cnt_thread.waiting_object = &waiting_object;

    if (!timeout) {
        // 1. Add to blocked list
        blocked_threads_.push_back(&cnt_thread);
        cnt_thread.status(thread_status::blocked);
        yield();
    } else {
        // 2. Add to delayed list
        delay_current_thread(*timeout);
    }
}

void scheduler::delay_current_thread(std::chrono::milliseconds timeout) noexcept {
    auto wakeup_time = current_time_ + timeout;
    auto &cnt_thread = current_thread();
    auto pivot = delayed_threads_.front();
    if (pivot) {
        thread *next;
        // 1. Find the last thread.wakeup_time <= cnt_thread.wakeup_time
        while ((next = delayed_threads_.next(pivot)) && next->wakeup_time_ <= wakeup_time) {
            pivot = next;
        }

        // 2. Add to wait list
        delayed_threads_.insert_after(pivot, &cnt_thread);
    } else {
        delayed_threads_.push_front(&cnt_thread);
    }
    cnt_thread.status(thread_status::delayed);
    yield();
}

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
    thread *next_thread;

    if (search_priority > cnt_priority || cnt_thread.status() != thread_status::running) {
        // 1. Higher thread ready or current thread blocked, search from max priority
        for (size_t i = search_priority; i > cnt_priority; i--) {
            auto &threads = ready_threads_[i];
            if (!threads.empty()) {
                next_thread = threads.front();
            }
        }
    } else {
        // 2. Round robin the same priority
        auto &threads = list_of(cnt_thread);
        next_thread = threads.size() == 1 ? &cnt_thread : threads.next(&cnt_thread);
    }

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

void scheduler::setup_idle_thread() noexcept {
    idle_thread_.initialize(thread_create_options{.process = &ke_process(),
                                                  .priority = thread_priority::idle,
                                                  .not_owned_stack = true,
                                                  .stack = idle_stack_,
                                                  .entry_point = ps_idle_thread_main});
    attach_thread(*idle_thread_);
    chino_current_threads[hal::arch_t::current_cpu_id()] = idle_thread_.get();
    idle_thread_->status(thread_status::running);
}

void scheduler::update_max_ready_priority(thread_priority priority) noexcept {
    if (max_ready_priority_ < priority) {
        max_ready_priority_ = priority;
    }
}

void scheduler::wakeup_delayed_threads() noexcept {
    auto current_time = current_time_;
    auto head = delayed_threads_.front();
    while (head && head->wakeup_time_ <= current_time) {
        auto wake = head;
        head = delayed_threads_.next(head);
        delayed_threads_.remove(wake);
        wake->status(thread_status::ready);
        list_of(*wake).push_back(wake);
        update_max_ready_priority(wake->priority());
    }
}

void scheduler::setup_next_system_tick() noexcept { hal::arch_t::enable_system_tick(system_tick_interval); }
