// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/units.h>
#include <cstddef>
#include <cstdint>

#if defined(__x86_64__)
#include <xmmintrin.h>
#endif

namespace chino::os::kernel::hal {
struct alignas(16) emulator_thread_context {
#if defined(__x86_64__)
    uintptr_t rax;
    uintptr_t rbx;
    uintptr_t rcx;
    uintptr_t rdx;
    uintptr_t rsp;
    uintptr_t rbp;
    uintptr_t rsi;
    uintptr_t rdi;
    uintptr_t r8;
    uintptr_t r9;
    uintptr_t r10;
    uintptr_t r11;
    uintptr_t r12;
    uintptr_t r13;
    uintptr_t r14;
    uintptr_t r15;

    uintptr_t rip;

    __m128 xmm[16];
#endif
};

struct emulator_method_table {
    void (*restore_context)(emulator_thread_context &);
};

class emulator_arch {
  public:
    inline static constexpr size_t min_page_size = 4 * KiB;

    static void initialize_method_table(emulator_method_table &mt) noexcept;

    static constexpr size_t current_cpu_id() noexcept { return 0; }

    [[noreturn]] static void restore_context(emulator_thread_context &context) noexcept;
};

using arch_t = emulator_arch;
using arch_thread_context_t = emulator_thread_context;
} // namespace chino::os::kernel::hal
