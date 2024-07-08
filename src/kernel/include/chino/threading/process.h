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
#include "thread.h"
#include <chino/ddk/list.h>
#include <chino/memory/heap_allocator.h>
#include <chino/object/handle_table.h>
#include <chino/ps.h>

namespace chino::threading
{
namespace details
{
    struct kprocess_checker;
}

struct kthread;

struct kprocess : public ob::object
{
    constexpr kprocess()
        : pid_(0), handle_table_(16), stdin_(handle_t::invalid()), stdout_(handle_t::invalid()), stderr_(handle_t::invalid()) {}

    void attach_new_thread(kthread &thread) noexcept;
    void detach_thread(kthread &thread) noexcept;

    // BEGIN LIST NODES, BE CAREFUL ABOUT THE OFFSETS !!!
    // END LIST NODES

    pid_t pid_;
    memory::heap_allocator heap_allocator_;
    ob::handle_table handle_table_;
    list_t_of_node(kthread::process_threads_entry_) threads_list_;

    sched_spinlock syncroot;
    handle_t stdin_, stdout_, stderr_;
};

namespace details
{
    struct kprocess_checker
    {
    };
}
}
