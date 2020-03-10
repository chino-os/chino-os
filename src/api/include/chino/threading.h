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
#include "error.h"
#include "object.h"
#include "result.h"
#include <atomic>
#include <string_view>

namespace chino::threading
{
enum class thread_priority : uint32_t
{
    lowest = 0,
    low = 1,
    normal = 2,
    high = 3,
    highest = 4
};

constexpr size_t thread_priority_count = 5;

enum class create_thread_flags : uint32_t
{
    none = 0,
    create_suspend = 1
};

class sched_spinlock
{
public:
    constexpr sched_spinlock() noexcept
        : taken_() {}

    sched_spinlock(sched_spinlock &) = delete;
    sched_spinlock &operator=(sched_spinlock &) = delete;

    bool try_lock() noexcept;
    void lock() noexcept;
    void unlock() noexcept;

private:
    std::atomic_flag taken_;
};

typedef int32_t (*chino_startup_t)();
typedef int32_t (*thread_start_t)(void *arg);

constexpr int32_t default_stack_size = 4096;

result<handle_t, error_code> create_process(chino_startup_t start, std::string_view command_lines, thread_priority prioriy = thread_priority::normal,
    int32_t stack_size = default_stack_size);

result<handle_t, error_code> create_thread(thread_start_t start, void *arg, thread_priority prioriy = thread_priority::normal,
    create_thread_flags flags = create_thread_flags::none, int32_t stack_size = default_stack_size);
}
