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
#include <array>
#include <chino/board/board.h>
#include <chino/threading/process.h>
#include <chino/threading/scheduler.h>

using namespace chino;
using namespace chino::threading;
using namespace chino::chip;
using namespace chino::arch;

namespace
{
std::array<scheduler, chip_t::processors_count> schedulers_;
}

scheduler &threading::current_sched() noexcept
{
    return schedulers_[arch_t::current_processor()];
}

kthread *threading::current_thread() noexcept
{
    return current_sched().current_thread();
}

kprocess *threading::current_process() noexcept
{
    auto thread = current_sched().current_thread();
    if (thread)
        return thread->owner_;
    return nullptr;
}

void scheduler::suspend() noexcept
{
    suspend_count_.fetch_add(1, std::memory_order_acq_rel);
}

void scheduler::resume() noexcept
{
    suspend_count_.fetch_sub(1, std::memory_order_acq_rel);
}

void scheduler::add_to_ready_list(kthread &thread) noexcept
{
    ready_list_[(size_t)thread.priority_].add_last(&thread.sched_entry_);

    kthread *expected = nullptr;
    selected_thread_.compare_exchange_weak(expected, &thread);
}

void scheduler::start() noexcept
{
    auto selected_thread = selected_thread_.load();
    assert(selected_thread);
    current_thread_ = selected_thread;
    arch_t::start_schedule(selected_thread->context_);
}
