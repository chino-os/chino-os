// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "process.h"
#include "../loaders/pe/pe_loader.h"
#include "../sched/scheduler.h"
#include <chino/os/kernel/io.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::ps;

void process::attach_thread(thread &thread) noexcept { threads_.push_back(&thread); }

result<void> ps::create_process(std::string_view filepath, lazy_construct<thread> &thread,
                                thread_create_options &options) noexcept {
    pe_loader loader;
    try_(loader.load(filepath));
    options.entry_point = reinterpret_cast<thread_start_t>(loader.entry());
    thread.construct(options);
    return ok();
}

result<void> ps::create_process(std::string_view filepath) noexcept { return err(error_code::not_implemented); }
