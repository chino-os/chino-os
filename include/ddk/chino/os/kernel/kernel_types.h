// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/os/processapi.h>
#include <chino/result.h>

namespace chino::os::kernel {
enum class syscall_number {
    yield,
};

namespace ps {
class thread;
typedef void (*thread_main_thunk_t)(thread_start_t entry_point, void *entry_arg);
} // namespace ps
} // namespace chino::os::kernel
