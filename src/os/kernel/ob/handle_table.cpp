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

result<std::pair<file *, int>> ob::alloc_handle(object &ob, access_mask granted_access) noexcept {
    auto &ht = ps::current_process().handle_table();
    try_var(handle, ht.allocate());
    std::construct_at(handle.first, ob, granted_access);
    return ok(handle);
}

result<file *> ob::reference_handle(int handle) noexcept {
    auto &ht = ps::current_process().handle_table();
    return ht.at(handle);
}

result<void> ob::close_handle(int handle) noexcept {
    auto &ht = ps::current_process().handle_table();
    return ht.free(handle);
}
