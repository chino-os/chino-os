// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/os/processapi.h>

namespace chino::os::kernel::ps {
typedef void (*thread_main_thunk_t)(void *thread, thread_start_t entry_point, void *entry_arg);
}
