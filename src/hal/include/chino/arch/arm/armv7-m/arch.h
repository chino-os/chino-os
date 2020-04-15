// MIT License
//
// Copyright (c) 2020 SunnyCase
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#pragma once
#include <chino/ddk/kernel.h>
#include <cstdint>
#include <gsl/gsl-lite.hpp>

namespace chino::arch
{
// No asynchronous interrupt is supported by this target
// Only save callee saved registers in context
struct armv7m_thread_context
{
    uintptr_t r4;
    uintptr_t r5;
    uintptr_t r6;
    uintptr_t r7;
    uintptr_t r8;
    uintptr_t r9;
    uintptr_t r10;
    uintptr_t r11;
    uintptr_t lr;
    uintptr_t sp;
    uintptr_t stack_bottom;
};

using thread_context_t = armv7m_thread_context;

struct armv7m_arch
{
    static constexpr size_t ALLOCATE_ALIGNMENT = 8;

    static uint32_t current_processor() noexcept { return 0; }
    static void yield_processor() noexcept;

    static uintptr_t disable_irq() noexcept;
    static void restore_irq(uintptr_t state) noexcept;

    static void init_thread_context(thread_context_t &context, gsl::span<uintptr_t> stack, kernel::thread_thunk_t start, void *arg0, void *arg1) noexcept;
    [[noreturn]] static void start_schedule(thread_context_t &context) noexcept;
    static void yield(thread_context_t &old_context, thread_context_t &new_context) noexcept;

    static void init_stack_check() noexcept;
};

using arch_t = armv7m_arch;
}
