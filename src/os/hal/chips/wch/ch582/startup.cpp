// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include <chino/compiler.h>
#include <chino/os/hal/arch.h>
#include <chino/os/hal/chip.h>
#include <chino/os/kernel/ke.h>
#include <chino/os/kernel/ps.h>
#include <mutex>

using namespace chino;
using namespace chino::os::hal;
using namespace chino::os::kernel;

namespace {
ps::irq_spin_lock debug_lock_;
} // namespace

extern "C" {
CHINO_KERNEL_API void hal_startup() noexcept{
    while (1) {
    }
}
}

void chip_t::debug_print(const char *str) noexcept {
    std::unique_lock<ps::irq_spin_lock> locker(debug_lock_);
    (void)str;
}
