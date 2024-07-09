// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include <Windows.h>
#include <chino/compiler.h>
#include <chino/os/hal/arch.h>
#include <chino/os/hal/chip.h>
#include <chino/os/kernel/ke.h>

using namespace chino;
using namespace chino::os::kernel;
using namespace chino::os::kernel::hal;

namespace {
char debug_buffer_[256];
ps::irq_spin_lock debug_lock_;
} // namespace

extern "C" {
CHINO_KERNEL_API void hal_startup(size_t memory_size) noexcept { emulator_arch::arch_startup(memory_size); }
}

void chip_t::debug_print(const char *format, ...) noexcept {
    va_list va;
    va_start(va, format);
    {
        std::unique_lock locker(debug_lock_);
        wvsprintfA(debug_buffer_, format, va);
        WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), debug_buffer_, lstrlenA(debug_buffer_), nullptr, nullptr);
    }
    va_end(va);
}
