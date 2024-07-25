// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include <Windows.h>
#include <chino/compiler.h>
#include <chino/os/hal/arch.h>
#include <chino/os/hal/chip.h>
#include <chino/os/kernel/ke.h>

using namespace chino;
using namespace chino::os::hal;
using namespace chino::os::kernel;

namespace {
ps::irq_spin_lock debug_lock_;
} // namespace

extern "C" {
CHINO_KERNEL_API void hal_startup(size_t memory_size) noexcept { emulator_arch::arch_startup(memory_size); }
}

void chip_t::debug_print(const char *str) noexcept {
    auto irq_state = debug_lock_.lock();
    OutputDebugStringA(str);
    debug_lock_.unlock(irq_state);
}
