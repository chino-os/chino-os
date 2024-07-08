// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "thread.h"
#include "../sched/scheduler.h"
#include "process.h"

using namespace chino::os::kernel;
using namespace chino::os::kernel::ps;

thread::thread(const thread_create_options &create_options) noexcept
    : context{.arch = hal::arch_t::initialize_thread_context(create_options.stack, thread_main_thunk, this,
                                                             create_options.entry_point, create_options.entry_arg)},
      flags_{.not_owned_stack = create_options.not_owned_stack, .priority = (uint32_t)create_options.priority},
      process_(create_options.process),
      stack_(create_options.stack) {
    process_->attach_thread(*this);
}

void thread::thread_main_thunk(void *thread, thread_start_t entry_point, void *entry_arg) noexcept {
    auto &this_thread = *reinterpret_cast<ps::thread *>(thread);
    (void)this_thread;
    entry_point(entry_arg);
    scheduler::current().detach_thread(this_thread);
    scheduler::current().yield();
    CHINO_UNREACHABLE();
}
