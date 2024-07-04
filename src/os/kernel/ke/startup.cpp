// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "../mm/memory_manager.h"
#include "../ps/sched/scheduler.h"
#include <chino/os/kernel/kernel.h>

using namespace chino::os::kernel;

namespace {
int ke_init_thread_main(void *arg) noexcept;

alignas(hal::cacheline_size) std::array<std::byte, sizeof(uintptr_t) * 128> init_stack_;
constinit ps::thread init_thread_;

int ke_init_thread_main(void *arg) noexcept {
    (void)arg;
    return 0;
}
} // namespace

extern "C" [[noreturn]] void CHINO_KERNEL_STARTUP(const boot_options &options) {
    // 1. Phase 0
    mm::initialize_phase0(options);

    // 2. Setup init thread
    init_thread_.initialize(init_stack_, ke_init_thread_main, nullptr);
    ps::scheduler::current().start_schedule(init_thread_);
}
