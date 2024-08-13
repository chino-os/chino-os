// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "thread.h"
#include "../sched/scheduler.h"
#include "chino/os/processapi.h"
#include "process.h"
#include <cstdint>

using namespace chino::os::kernel;
using namespace chino::os::kernel::ps;

thread::thread(const thread_create_options &create_options) noexcept
    : stack_top(initialize_thread_stack(create_options)),
      stack_bottom(create_options.stack.data()),
#ifdef CHINO_EMULATOR
      emulator_handle(hal::arch_t::initialize_thread_handle(*this)),
#endif
      flags_{.not_owned_stack = create_options.not_owned_stack,
             .priority = (uint32_t)create_options.priority,
             .status = (uint32_t)thread_status::ready},
      process_(create_options.process) {
    process_->attach_thread(*this);
    scheduler::current().attach_thread(*this);
}

void thread::thread_main_thunk(thread_start_t entry_point, void *entry_arg) noexcept {
    auto &this_thread = current_thread();
    entry_point(entry_arg);
    scheduler::current().detach_thread(this_thread);
    yield();
    CHINO_UNREACHABLE();
}

uintptr_t *thread::initialize_thread_stack(const thread_create_options &create_options) noexcept {
    auto stack_top = create_options.stack.data() + create_options.stack.size();
    hal::arch_t::initialize_thread_stack(stack_top, thread_main_thunk, create_options.entry_point,
                                         create_options.entry_arg);
    return stack_top;
}
