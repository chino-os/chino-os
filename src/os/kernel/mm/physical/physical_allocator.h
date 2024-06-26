// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <atomic>
#include <chino/os/kernel/hal/cpu/cpu.h>
#include <chino/os/kernel/kernel.h>
#include <chino/result.h>
#include <numeric>

namespace chino::os::kernel::mm::physical_allocator {
void initialize_phase0(const boot_options &options);
} // namespace chino::os::kernel::mm::physical::physical_allocator
