// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <atomic>
#include <chino/os/kernel/hal/cpu/cpu.h>
#include <chino/os/kernel/kernel.h>
#include <chino/result.h>
#include <numeric>

namespace chino::os::kernel::mm {
class physical_allocator {
  public:
    static void initialize_phase0(const boot_options &options) noexcept;
};
} // namespace chino::os::kernel::mm
