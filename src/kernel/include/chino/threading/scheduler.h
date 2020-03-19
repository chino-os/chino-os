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
#include "process.h"
#include "thread.h"
#include <chino/object.h>

namespace chino::threading
{
class scheduler
{
public:
    constexpr scheduler() noexcept
        : suspend_count_(0), selected_thread_(nullptr), current_thread_(nullptr), ready_list_ {}, idle_thread_ {}
    {
    }

    void suspend() noexcept;
    void resume() noexcept;

    kthread *current_thread() const noexcept { return current_thread_.load(); }
    void add_to_ready_list(kthread &thread) noexcept;

    [[noreturn]] void start() noexcept;

private:
    void init_idle_thread() noexcept;

private:
    std::atomic<uint32_t> suspend_count_;
    std::array<list_t_of_node(kthread::sched_entry_), THREAD_PRIORITY_COUNT> ready_list_;
    kthread *selected_thread_;
    std::atomic<kthread *> current_thread_;
    ob::static_object<kthread> idle_thread_;
    std::array<uintptr_t, IDLE_STACK_SIZE / sizeof(uintptr_t)> idle_stack_;
    irq_spinlock selected_thread_lock_;
};

struct sched_lock
{
    sched_lock() noexcept;
    ~sched_lock();
};

scheduler &current_sched() noexcept;
kprocess *current_process() noexcept;
kthread *current_thread() noexcept;
}
