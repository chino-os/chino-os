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
#include <atomic>
#include <board.h>
#include <cassert>
#include <chino/arch/arm/armv7-m/arch.h>
#include <chino/arch/arm/armv7-m/platform.h>
#include <chino/arch/arm/armv7-m/core_debug.h>
#include <chino/arch/arm/armv7-m/dwt.h>

using namespace chino::arch;
using namespace chino::chip;
using namespace chino::kernel;

extern "C"
{
    extern  void arm_start_schedule(thread_context_t *ctx);
    extern  void arm_yield(thread_context_t *old_ctx, thread_context_t *new_ctx);
    extern  void arm_thread_thunk();
}

namespace
{
void setup_stack_check(thread_context_t &context) noexcept
{
    dwt::slot_set_address(3, context.stack_bottom);
    dwt::slot_set_mask(3, 0);
    dwt::slot_set_function(3, dwt::slot_function_t::watchpoint_rw, dwt::data_size_t::word);
}
}

void armv7m_arch::yield_processor() noexcept
{
    asm volatile ("yield");
}

uintptr_t armv7m_arch::disable_irq() noexcept
{
    uintptr_t old;
    asm volatile
    (
        "mrs %0, primask \n"
        "cpsid i"
        : "=r"(old)
    );
    return old;
}

void armv7m_arch::restore_irq(uintptr_t state) noexcept
{
    asm volatile ("msr primask, %0" :: "r"(state));
}

void armv7m_arch::init_thread_context(thread_context_t &context, gsl::span<uintptr_t> stack, thread_thunk_t start, void *arg0, void *arg1) noexcept
{
    // r4: start
    // r5: arg0
    // r6: arg1
    context.r4 = uintptr_t(start);
    context.r5 = uintptr_t(arg0);
    context.r6 = uintptr_t(arg1);
    context.stack_bottom = uintptr_t(stack.data());

    auto *top = stack.end();
    // Align as 8
    while ((uintptr_t(top) & 4) == 1)
        *--top = 0;

    context.lr = uintptr_t(arm_thread_thunk) | 1;   // thumb
    context.sp = uintptr_t(top);
}

void armv7m_arch::start_schedule(thread_context_t &context) noexcept
{
    setup_stack_check(context);
    arm_start_schedule(&context);
}

void armv7m_arch::yield(thread_context_t &old_context, thread_context_t &new_context) noexcept
{
    setup_stack_check(new_context);
    arm_yield(&old_context, &new_context);
}

void armv7m_arch::init_stack_check() noexcept
{
    core_debug::monitor_enable();
    core_debug::trace_enable();
}
