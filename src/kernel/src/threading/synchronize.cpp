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
#include <chino/board/board.h>
#include <chino/threading.h>
#include <chino/threading/scheduler.h>

using namespace chino;
using namespace chino::arch;
using namespace chino::threading;

bool sched_spinlock::try_lock() noexcept
{
    // suspend sched
    auto &sched = current_sched();
    sched.suspend();

    // not taken, resume sched
    if (taken_.test_and_set(std::memory_order_acq_rel))
    {
        sched.resume();
        return false;
    }

    return true;
}

void sched_spinlock::lock() noexcept
{
    while (true)
    {
        // take the lock
        if (!try_lock())
            break;

        // spin
        arch_t::yield_processor();
    }
}

void sched_spinlock::unlock() noexcept
{
    taken_.clear();
    // resume sched
    current_sched().resume();
}

bool irq_spinlock::try_lock() noexcept
{
    // disable irq
    auto state = arch_t::disable_irq();

    // not taken, restore irq
    if (taken_.test_and_set(std::memory_order_acq_rel))
    {
        arch_t::restore_irq(state);
        return false;
    }

    irq_state_ = state;
    return true;
}

void irq_spinlock::lock() noexcept
{
    while (true)
    {
        // take the lock
        if (!try_lock())
            break;

        // spin
        arch_t::yield_processor();
    }
}

void irq_spinlock::unlock() noexcept
{
    taken_.clear();
    // restore irq
    arch_t::restore_irq(irq_state_);
}

sched_lock::sched_lock() noexcept
{
    // suspend sched
    auto &sched = current_sched();
    sched.suspend();
}

sched_lock::~sched_lock()
{
    // resume sched
    current_sched().resume();
}
