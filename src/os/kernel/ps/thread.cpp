// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "thread.h"

using namespace chino::os::kernel;
using namespace chino::os::kernel::ps;

void thread::initialize(std::span<std::byte> stack, thread_start_t entry_point, void *entry_arg) noexcept {
    flags_.not_owned_stack = 1;
    context.arch = hal::arch_t::initialize_thread_context(stack, thread_main_thunk, this, entry_point, entry_arg);
}

void thread::thread_main_thunk(void *thread, thread_start_t entry_point, void *entry_arg) noexcept {
    auto &this_thread = *reinterpret_cast<ps::thread *>(thread);
    (void)this_thread;
    entry_point(entry_arg);
    while (1)
        ;
}
