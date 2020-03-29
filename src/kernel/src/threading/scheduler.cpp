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
#include <board.h>
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

static uint32_t idle_main(scheduler &sched) noexcept;

scheduler &threading::current_sched() noexcept
{
    return schedulers_[arch_t::current_processor()];
}

kthread *threading::current_thread() noexcept
{
    return current_sched().current_thread();
}

kprocess &threading::current_process() noexcept
{
    auto thread = current_sched().current_thread();
    if (thread)
        return *thread->owner_;
    return kernel::kernel_process();
}

void scheduler::suspend() noexcept
{
    suspend_count_.fetch_add(1, std::memory_order_acq_rel);
}

void scheduler::resume() noexcept
{
    if (suspend_count_.fetch_sub(1, std::memory_order_acq_rel) == 1)
        yield_if_needed();
}

void scheduler::add_to_ready_list(kthread &thread) noexcept
{
    ready_list_[(size_t)thread.priority_].add_last(&thread.sched_entry_);

    std::unique_lock lock(selected_thread_lock_);
    if (!selected_thread_ || selected_thread_->priority_ < thread.priority_)
        selected_thread_ = &thread;
}

void scheduler::init_idle_thread() noexcept
{
    idle_thread_.body.priority_ = thread_priority::lowest;
    idle_thread_.body.init_stack(idle_stack_, (thread_start_t)idle_main, this);
    kernel::kernel_process().attach_new_thread(idle_thread_.body);
    current_sched().add_to_ready_list(idle_thread_.body);
}

void scheduler::start() noexcept
{
    init_idle_thread();

    // No lock is needed because of irq is disabled in init
    auto selected_thread = selected_thread_;
    assert(selected_thread);
    current_thread_ = selected_thread;
    arch_t::start_schedule(selected_thread->context_);
}

void scheduler::select_highest_thread() noexcept
{
    for (auto it = ready_list_.rbegin(); it != ready_list_.rend(); ++it)
    {
        std::unique_lock lock(it->syncroot());
        if (auto first = it->first_nolock())
        {
            selected_thread_ = first->owner();
            return;
        }
    }

    panic("No thread available");
}

void scheduler::on_thread_exit(kthread &thread) noexcept
{
    sched_lock lock;
    auto &sched_entry = thread.sched_entry_;
    sched_entry.list->remove(&sched_entry);
    thread.owner_->detach_thread(thread);
    destroy_list_.add_last(&thread.sched_entry_);
    if (current_thread_ == &thread)
        select_highest_thread();
}

void scheduler::yield_if_needed() noexcept
{
    // If started
    if (idle_thread_.body.owner_)
    {
        auto selected_thread = selected_thread_;
        assert(selected_thread);
        auto old_thread = current_thread_.load();
        if (selected_thread != old_thread)
        {
            current_thread_ = selected_thread;
            arch_t::yield(old_thread->context_, selected_thread->context_);
        }
    }
}

uint32_t idle_main(scheduler &sched) noexcept
{
    while (true)
    {
    }

    return 0;
}

void threading::exit_thread(uint32_t exit_code)
{
    {
        sched_lock lock;
        auto thread = current_thread();
        assert(thread);
        thread->exit_code_ = exit_code;
        current_sched().on_thread_exit(*thread);
    }

    // Should not reach here
    while (1)
        ;
}
