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
#include <chino/ddk/ke.h>
#include <cstdint>
#include <gsl/gsl-lite.hpp>
#include <xmmintrin.h>

namespace chino::arch
{
// No asynchronous interrupt is supported by this target
// Only save callee saved registers in context
struct win32_thread_context
{
    uintptr_t rbx;
    uintptr_t rbp;
    uintptr_t rdi;
    uintptr_t rsi;
    uintptr_t rsp;
    uintptr_t r12;
    uintptr_t r13;
    uintptr_t r14;
    uintptr_t r15;
    uintptr_t reserved0;

    __m128 xmm6;
    __m128 xmm7;
    __m128 xmm8;
    __m128 xmm9;
    __m128 xmm10;
    __m128 xmm11;
    __m128 xmm12;
    __m128 xmm13;
    __m128 xmm14;
    __m128 xmm15;

    uintptr_t stack_low;
    uintptr_t stack_high;
};

using thread_context_t = win32_thread_context;

struct win32_arch
{
    static constexpr size_t ALLOCATE_ALIGNMENT = 16;

    static uint32_t current_processor() noexcept { return 0; }
    static void yield_processor() noexcept;

    static uintptr_t disable_irq() noexcept;
    static void restore_irq(uintptr_t state) noexcept;

    static void init_thread_context(thread_context_t &context, gsl::span<uintptr_t> stack, kernel::thread_thunk_t start, void *arg0, void *arg1) noexcept;
    [[noreturn]] static void start_schedule(thread_context_t &context) noexcept;
    static void yield(thread_context_t &old_context, thread_context_t &new_context) noexcept;

    static void init_stack_check() noexcept;
};

using arch_t = win32_arch;
}
