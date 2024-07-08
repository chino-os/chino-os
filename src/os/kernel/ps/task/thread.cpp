// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "thread.h"
#include "../sched/scheduler.h"
#include "process.h"

using namespace chino::os::kernel;
using namespace chino::os::kernel::ps;

thread::thread(const thread_create_options &create_options) noexcept
    : stack_top(initialize_thread_stack(create_options)),
      flags_{.not_owned_stack = create_options.not_owned_stack, .priority = (uint32_t)create_options.priority},
      process_(create_options.process),
      stack_(create_options.stack) {
    process_->attach_thread(*this);
}

void thread::thread_main_thunk(thread_start_t entry_point, void *entry_arg) noexcept {
    auto &this_thread = scheduler::current_thread();
    entry_point(entry_arg);
    scheduler::current().detach_thread(this_thread);
    scheduler::current().yield();
    CHINO_UNREACHABLE();
}

uintptr_t *thread::initialize_thread_stack(const thread_create_options &create_options) noexcept {
    auto stack_top = create_options.stack.data() + create_options.stack.size();
    *--stack_top = 0; // Avoid unwind
    *--stack_top = (uintptr_t)create_options.entry_arg;
    *--stack_top = (uintptr_t)create_options.entry_point;
    *--stack_top = (uintptr_t)thread_main_thunk;
    hal::arch_t::initialize_thread_stack(stack_top);
    return stack_top;
}
