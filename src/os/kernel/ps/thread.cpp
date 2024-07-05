// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "thread.h"
#include "process.h"
#include "sched/scheduler.h"

using namespace chino::os::kernel;
using namespace chino::os::kernel::ps;

void thread::initialize(ps::process &process, std::span<std::byte> stack, thread_start_t entry_point,
                        void *entry_arg) noexcept {
    flags_.not_owned_stack = 1;
    process_ = &process;
    process.attach_thread(*this);
    stack_ = stack;
    context.arch = hal::arch_t::initialize_thread_context(stack, thread_main_thunk, this, entry_point, entry_arg);
}

void thread::thread_main_thunk(void *thread, thread_start_t entry_point, void *entry_arg) noexcept {
    auto &this_thread = *reinterpret_cast<ps::thread *>(thread);
    (void)this_thread;
    entry_point(entry_arg);
    scheduler::current().detach_thread(this_thread);
    scheduler::current().yield();
    CHINO_UNREACHABLE();
}
