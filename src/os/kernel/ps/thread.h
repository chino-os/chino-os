// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "chino/os/kernel/object_kind.h"
#include <atomic>
#include <chino/instrusive_list.h>
#include <chino/os/kernel/hal/arch.h>
#include <chino/os/kernel/object.h>
#include <chino/os/kernel/threading.h>
#include <span>

namespace chino::os::kernel::ps {
class process;

struct thread_context {
    hal::arch_thread_context_t arch;
};

struct thread_flags {
    uint32_t not_owned_stack : 1;
    uint32_t priority : 3;
};

class thread : public object {
    CHINO_DEFINE_KERNEL_OBJECT_KIND(object, object_kind_thread);

  public:
    constexpr thread() noexcept : context{}, flags_{} {};

    void initialize(ps::process &process, std::span<std::byte> stack, thread_start_t entry_point,
                    void *entry_arg) noexcept;

    thread_priority priority() const noexcept { return (thread_priority)flags_.priority; }
    ps::process &process() const noexcept { return *process_.get(); }

  private:
    [[noreturn]] static void thread_main_thunk(void *thread, thread_start_t entry_point, void *entry_arg) noexcept;

  public:
    intrusive_list_node scheduler_list_node;
    intrusive_list_node process_list_node;
    thread_context context;

  private:
    thread_flags flags_;
    object_ptr<ps::process> process_;
    std::span<std::byte> stack_;
};

object_ptr<thread> create_thread();
} // namespace chino::os::kernel::ps
