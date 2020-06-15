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
#include <chino/arch/win32/arch.h>
#include <chino/arch/win32/target.h>
#include <intrin.h>

using namespace chino::arch;
using namespace chino::chip;
using namespace chino::kernel;

extern "C"
{
    extern void win32_start_schedule(thread_context_t *ctx);
    extern void win32_yield(thread_context_t *old_ctx, thread_context_t *new_ctx);
    extern void win32_thread_thunk();
}

static HANDLE processor_handle[chip_t::processors_count];
static CONTEXT processor_ctx[chip_t::processors_count];
static std::atomic<uintptr_t> irq_state = 0;

struct _TEB
{
    uintptr_t reserved0;
    uintptr_t stack_high;
    uintptr_t stack_low;
};

namespace
{
inline void set_bits(DWORD64 &dw, uint64_t low_bit, uint64_t bits, uint64_t new_value)
{
    uint64_t mask = (1ULL << bits) - 1; // e.g. 1 becomes 0001, 2 becomes 0011, 3 becomes 0111
    dw = (dw & ~(mask << low_bit)) | (new_value << low_bit);
}

void setup_stack_check(thread_context_t &context) noexcept
{
    auto hart = arch_t::current_processor();
    auto &ctx = processor_ctx[hart];

    auto teb = NtCurrentTeb();
    teb->stack_low = context.stack_low;
    teb->stack_high = context.stack_high;

    // Setup stack checker
    ctx.Dr0 = context.stack_low;
    set_bits(ctx.Dr7, 16, 2, 0b11);
    set_bits(ctx.Dr7, 24, 2, 0b10);
    set_bits(ctx.Dr7, 0, 1, 1);
    //assert(SetThreadContext(processor_handle[hart], &ctx));
}
}

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
    context.stack_low = uintptr_t(stack.begin());
    context.stack_high = uintptr_t(stack.end());

    auto *top = stack.end();
    // Align as 16
    if ((uintptr_t(top) & 8) == 0)
    {
        *--top = 0;
    }
    else
    {
        // Zero ret
        *--top = 0;
        *--top = 0;
    }

    *--top = uintptr_t(win32_thread_thunk);
    context.rsp = uintptr_t(top);
}

void win32_arch::start_schedule(thread_context_t &context) noexcept
{
    setup_stack_check(context);
    win32_start_schedule(&context);
}

void win32_arch::yield(thread_context_t &old_context, thread_context_t &new_context) noexcept
{
    setup_stack_check(new_context);
    win32_yield(&old_context, &new_context);
}

void win32_arch::init_stack_check() noexcept
{
    auto hart = current_processor();
    processor_handle[hart] = GetCurrentThread();
    auto &ctx = processor_ctx[hart];

    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    assert(GetThreadContext(processor_handle[hart], &ctx));
}
