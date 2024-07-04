// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include <chino/compiler.h>
#include <chino/os/kernel/hal/archs/emulator/arch.h>

using namespace chino::os::kernel;
using namespace chino::os::kernel::hal;

extern "C" {
[[noreturn]] extern void emulator_restore_context(emulator_thread_context *context) noexcept;
}

namespace {
emulator_method_table emu_mt_;
}

void emulator_arch::initialize_method_table(emulator_method_table &mt) noexcept { emu_mt_ = mt; }

void emulator_arch::restore_context(emulator_thread_context &context) noexcept { emulator_restore_context(&context); }
