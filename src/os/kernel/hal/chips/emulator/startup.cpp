// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include <chino/compiler.h>
#include <chino/os/kernel/hal/arch.h>
#include <chino/os/kernel/hal/chip.h>
#include <chino/os/kernel/ke.h>

using namespace chino;
using namespace chino::os::kernel;
using namespace chino::os::kernel::hal;

extern "C" {
CHINO_KERNEL_API void hal_startup(size_t memory_size) noexcept { emulator_arch::arch_startup(memory_size); }
}
