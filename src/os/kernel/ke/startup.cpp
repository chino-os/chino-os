// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "../io/io_manager.h"
#include "../mm/memory_manager.h"
#include "../ps/sched/scheduler.h"
#include "../ps/task/process.h"
#include "ke_services.h"
#include <chino/os/hal/chip.h>
#include <chino/os/kernel/ke.h>

using namespace chino::os;
using namespace chino::os::kernel;

alignas(hal::cacheline_size) static std::array<uintptr_t, 128 * 1024> init_stack_;
static constinit ps::process ke_process_;
static constinit static_object<ps::thread> init_thread_;

[[noreturn]] static int ke_init_system(void *arg) noexcept;
[[noreturn]] static void ke_idle_loop() noexcept;

ps::process &chino::os::kernel::ke_process() noexcept { return ke_process_; }

void ke_startup(const boot_options &options) noexcept {
    // 1. Phase 0
    mm::initialize_phase0(options);
    ps::scheduler::current().initialize_phase0();

    // 2. Setup init thread
    init_thread_.construct(ps::thread_create_options{.process = &ke_process_,
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
    io::initialize_phase1(options).expect("Initialize IO system failed.");

    // 2. Initialize user land
    initialize_ke_services().expect("Initialize Ke Services failed.");

    // 3. Launch shell
    ps::create_process("/dev/fs0/chino/system/chino.shell.sh.exe").expect("Launch shell failed.");
    ke_idle_loop();
}

void ke_idle_loop() noexcept {
    while (true) {
        hal::arch_t::yield_cpu();
    }
}
