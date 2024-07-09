// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "chino/os/kernel/object_kind.h"
#include <atomic>
#include <chino/instrusive_list.h>
#include <chino/os/kernel/hal/arch.h>
#include <chino/os/kernel/object.h>
#include <chino/os/kernel/ps.h>
#include <span>

namespace chino::os::kernel::ps {
class process;
class scheduler;

struct thread_flags {
    uint32_t not_owned_stack : 1;
    uint32_t priority : 3;
    uint32_t status : 3;
    uint32_t scheduled : 1;
};

struct thread_create_options {
    ps::process *process;
    thread_priority priority = thread_priority::normal;
    bool not_owned_stack = false;
    std::span<uintptr_t> stack;
    thread_start_t entry_point;
    void *entry_arg = nullptr;
};

class thread : public object {
    CHINO_DEFINE_KERNEL_OBJECT_KIND(object, object_kind_thread);

  public:
    thread(const thread_create_options &create_options) noexcept;

    ps::process &process() const noexcept { return *process_.get(); }
    thread_priority priority() const noexcept { return (thread_priority)flags_.priority; }

    thread_status status() const noexcept { return (thread_status)flags_.status; }
    void status(thread_status value) noexcept { flags_.status = (uint32_t)value; }

    bool set_scheduled() noexcept {
        if (flags_.scheduled) {
            return true;
        } else {
            flags_.scheduled = 1;
            return false;
        }
    }

    ps::scheduler *scheduler() const noexcept { return scheduler_.load(std::memory_order_acquire); }
    void scheduler(ps::scheduler *value) noexcept { scheduler_.store(value, std::memory_order_release); }

  private:
    [[noreturn]] static void thread_main_thunk(thread_start_t entry_point, void *entry_arg) noexcept;
    static uintptr_t *initialize_thread_stack(const thread_create_options &create_options) noexcept;

  public:
    uintptr_t *stack_top;
    uintptr_t *stack_bottom;

    intrusive_list_node scheduler_list_node;
    intrusive_list_node process_list_node;

    object_ptr<waitable_object> waiting_object;
    intrusive_list_node waiting_list_node;

    std::chrono::milliseconds wakeup_time_;

  private:
    thread_flags flags_;
    object_ptr<ps::process> process_;
    std::atomic<ps::scheduler *> scheduler_;
};

object_ptr<thread> create_thread();
} // namespace chino::os::kernel::ps
