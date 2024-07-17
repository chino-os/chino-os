// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "handle_table.h"
#include "../ps/sched/scheduler.h"
#include "../ps/task/process.h"
#include <chino/os/kernel/ob.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::ob;

result<int> ob::insert_handle(file &&file) noexcept {
    auto &ht = ps::scheduler::current_process().handle_table();
    try_var(handle, ht.allocate());
    std::construct_at(handle.first, std::move(file));
    return ok(handle.second);
}

result<file *> ob::reference_handle(int handle) noexcept {
    auto &ht = ps::scheduler::current_process().handle_table();
    return ht.at(handle);
}

result<void> ob::close_handle(int handle) noexcept {
    auto &ht = ps::scheduler::current_process().handle_table();
    return ht.free(handle);
}
