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

#define SHELL_NAME "sh" // "sh"

alignas(hal::cacheline_size) static std::array<uintptr_t, 128 * 1024> idle_stack_;
static constinit lazy_construct<ps::thread> idle_thread_;

alignas(hal::cacheline_size) static std::array<uintptr_t, 128 * 1024> init_stack_;
static constinit ps::process ke_process_;
static constinit lazy_construct<ps::thread> init_thread_;

alignas(hal::cacheline_size) static std::array<uintptr_t, 128 * 1024> sh_stack_;
static constinit ps::process sh_process_;
static constinit lazy_construct<ps::thread> sh_thread_;

[[noreturn]] static int ke_init_system(void *arg) noexcept;
[[noreturn]] static int ke_idle_loop(void *arg) noexcept;

ps::process &chino::os::kernel::ke_process() noexcept { return ke_process_; }

bool chino::os::kernel::is_io_worker_thread(ps::thread &thread) noexcept { return &thread == init_thread_.get(); }

void ke_startup(const boot_options &options) noexcept {
    // 1. Phase 0
    mm::initialize_phase0(options);
    ps::scheduler::current().initialize_phase0();

    // 2. Setup idle thread
    idle_thread_.construct(ps::thread_create_options{.process = &ke_process_,
                                                     .priority = thread_priority::idle,
                                                     .not_owned_stack = true,
                                                     .stack = idle_stack_,
                                                     .entry_point = ke_idle_loop,
                                                     .entry_arg = nullptr});

    // 3. Setup init thread
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
    ps::thread_create_options sh_create_options{
        .process = &sh_process_, .priority = thread_priority::normal, .not_owned_stack = true, .stack = sh_stack_};
    ps::create_process("/bin/" SHELL_NAME ".exe", sh_thread_, sh_create_options).expect("Launch shell failed.");

    // 4. Run IO worker
    io::io_worker_main(nullptr);
}

int ke_idle_loop(void *arg) noexcept {
    while (true) {
        hal::arch_t::yield_cpu();
    }
}
