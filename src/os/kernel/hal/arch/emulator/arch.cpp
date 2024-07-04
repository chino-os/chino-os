// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include <chino/compiler.h>
#include <chino/os/kernel/hal/archs/emulator/arch.h>

using namespace chino::os::kernel;
using namespace chino::os::kernel::hal;

extern "C" {
[[noreturn]] extern void emulator_restore_context(emulator_thread_context *context) noexcept;
[[noreturn]] extern void emulator_start_schedule(emulator_thread_context *context) noexcept;
}

namespace {
emulator_method_table emu_mt_;
}

void emulator_arch::initialize_method_table(emulator_method_table &mt) noexcept { emu_mt_ = mt; }

emulator_thread_context emulator_arch::initialize_thread_context(std::span<std::byte> stack,
                                                                 thread_main_thunk_t thread_thunk, void *thread,
                                                                 ps::thread_start_t entry_point,
                                                                 void *entry_arg) noexcept {
    return emulator_thread_context{.rcx = (uintptr_t)thread,
                                   .rdx = (uintptr_t)entry_point,
                                   .rsp = (uintptr_t)stack.data() + stack.size_bytes(),
                                   .r8 = (uintptr_t)entry_arg,
                                   .rip = (uintptr_t)thread_thunk};
}

void emulator_arch::restore_context(emulator_thread_context &context) noexcept { emulator_restore_context(&context); }

void emulator_arch::start_schedule(emulator_thread_context &context) noexcept { emulator_start_schedule(&context); }
