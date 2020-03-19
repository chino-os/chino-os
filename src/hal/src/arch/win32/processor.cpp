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
#include "target.h"
#include <Windows.h>
#include <atomic>
#include <cassert>
#include <chino/arch/win32/arch.h>

using namespace chino::arch;
using namespace chino::kernel;

extern "C"
{
    extern void win32_start_schedule(thread_context_t *ctx);
    extern void win32_thread_thunk();
}

static std::atomic<uintptr_t> irq_state = 0;

void win32_arch::yield_processor() noexcept
{
    YieldProcessor();
}

uintptr_t win32_arch::disable_irq() noexcept
{
    return irq_state.exchange(0);
}

void win32_arch::restore_irq(uintptr_t state) noexcept
{
    irq_state.store(state, std::memory_order_release);
}

void win32_arch::init_thread_context(thread_context_t &context, gsl::span<uintptr_t> stack, thread_thunk_t start, void *arg0, void *arg1) noexcept
{
    // RBX: start
    // RDI: arg0
    // RSI: arg1
    context.rbx = uintptr_t(start);
    context.rdi = uintptr_t(arg0);
    context.rsi = uintptr_t(arg1);

    auto *top = stack.end();
    *--top = uintptr_t(win32_thread_thunk);
    context.rsp = uintptr_t(top);
}

void win32_arch::start_schedule(thread_context_t &context) noexcept
{
    win32_start_schedule(&context);
}
