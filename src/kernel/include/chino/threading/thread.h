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
#include <board.h>
#include <chino/ddk/list.h>
#include <chino/ddk/object.h>
#include <chino/threading.h>
#include <gsl/gsl-lite.hpp>

namespace chino::threading
{
namespace details
{
    struct kthread_checker;
}

struct kprocess;

struct kthread : public ob::object
{
    static constexpr ptrdiff_t PROCESS_THREAD_ENTRY_OFFSET = 0;
    static constexpr ptrdiff_t SCHED_ENTRY_OFFSET = PROCESS_THREAD_ENTRY_OFFSET + VOID_LIST_NODE_SIZE;

    kthread() = default;

    void init_stack(gsl::span<uintptr_t> stack, thread_start_t start, void *arg) noexcept;

    friend struct details::kthread_checker;

    // BEGIN LIST NODES, BE CAREFUL ABOUT THE OFFSETS !!!
    list_node<kthread, void, PROCESS_THREAD_ENTRY_OFFSET> process_threads_entry_;
    list_node<kthread, void, SCHED_ENTRY_OFFSET> sched_entry_;
    // END LIST NODES

    kprocess *owner_ = nullptr;
    tid_t tid_;
    thread_priority priority_;
    std::atomic<uint32_t> exit_code_;
    gsl::span<uintptr_t> stack_;
    arch::thread_context_t context_;
};

namespace details
{
    struct kthread_checker
    {
        static_assert(offsetof(kthread, process_threads_entry_) == kthread::PROCESS_THREAD_ENTRY_OFFSET);
    };
}
}
