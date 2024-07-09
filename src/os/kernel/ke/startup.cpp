// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "../io/io_manager.h"
#include "../mm/memory_manager.h"
#include "../ps/sched/scheduler.h"
#include "../ps/task/process.h"
#include <chino/os/hal/chip.h>
#include <chino/os/kernel/ke.h>

using namespace chino::os;
using namespace chino::os::kernel;

namespace {
alignas(hal::cacheline_size) std::array<uintptr_t, 512> init_stack_;
constinit static_object<ps::process> ke_process_;
constinit static_object<ps::thread> init_thread_;
} // namespace

[[noreturn]] static int ke_init_system(void *arg) noexcept;
[[noreturn]] static void ke_idle_loop() noexcept;

ps::process &chino::os::kernel::ke_process() noexcept { return *ke_process_; }

void ke_startup(const boot_options &options) noexcept {
    // 1. Phase 0
    mm::initialize_phase0(options);
    ke_process_.initialize();
    ps::scheduler::current().initialize_phase0();

    // 2. Setup init thread
    init_thread_.initialize(ps::thread_create_options{.process = ke_process_.get(),
                                                      .priority = thread_priority::highest,
                                                      .not_owned_stack = true,
                                                      .stack = init_stack_,
                                                      .entry_point = ke_init_system,
                                                      .entry_arg = (void *)&options});
    ps::scheduler::current().start_schedule(*init_thread_);
}

int ke_init_system(void *pv_options) noexcept {
    // 1. Phase 1
    auto &options = *reinterpret_cast<const boot_options *>(pv_options);
    hal::chip_t::debug_print("ke_init_system\n");
    io::initialize_phase1(options);

    ke_idle_loop();
}

void ke_idle_loop() noexcept {
    while (true) {
        hal::arch_t::yield_cpu();
    }
}
