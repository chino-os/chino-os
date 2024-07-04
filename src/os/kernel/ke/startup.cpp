// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "../mm/memory_manager.h"
#include "../ps/sched/scheduler.h"
#include <chino/os/kernel/kernel.h>

using namespace chino::os::kernel;

extern "C" [[noreturn]] void CHINO_KERNEL_STARTUP(const boot_options &options) {
    // 1. Phase 0
    mm::initialize_phase0(options);

    ps::scheduler::current().start_schedule();
}
