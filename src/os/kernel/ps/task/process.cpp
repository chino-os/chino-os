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

namespace {
result<void> create_process_worker(std::string_view filepath) {
    pe_loader loader;
    try_(loader.load(filepath));
    auto entry = reinterpret_cast<thread_start_t>(loader.entry());
    return entry(nullptr) == 0 ? ok() : err(error_code::fail);
}

int create_process_thunk(void *arg) {
    auto filepath = (const char *)arg;
    return (int)create_process_worker(filepath).unwrap_err();
}
} // namespace

void process::attach_thread(thread &thread) noexcept { threads_.push_back(&thread); }

result<void> ps::create_process(std::string_view filepath, lazy_construct<thread> &thread,
                                thread_create_options &options) noexcept {
    options.entry_point = create_process_thunk;
    options.entry_arg = (void *)filepath.data();
    thread.construct(options);
    return ok();
}

result<void> ps::create_process(std::string_view filepath) noexcept { return err(error_code::not_implemented); }
