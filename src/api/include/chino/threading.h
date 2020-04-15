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
#include "chinodef.h"
#include <atomic>
#include <chino/error.h>
#include <chino/result.h>
#include <string_view>

namespace chino::threading
{
typedef uint16_t pid_t;
inline constexpr pid_t INVALID_PID = -1;

typedef uint32_t tid_t;
inline constexpr tid_t INVALID_TID = -1;

enum class thread_priority : uint32_t
{
    lowest = 0,
    low = 1,
    normal = 2,
    high = 3,
    highest = 4
};

inline constexpr size_t THREAD_PRIORITY_COUNT = 5;
inline constexpr size_t DEFAULT_MUTEX_SPIN_COUNT = 500;

enum class create_thread_flags : uint32_t
{
    none = 0,
    create_suspend = 1
};

typedef uint32_t (*chino_startup_t)();
typedef uint32_t (*thread_start_t)(void *arg);

inline constexpr int32_t DEFAULT_STACK_SIZE = sizeof(uintptr_t) * 128;

result<handle_t, error_code> create_process(chino_startup_t start, std::string_view command_lines, thread_priority prioriy = thread_priority::normal,
    int32_t stack_size = DEFAULT_STACK_SIZE);

result<handle_t, error_code> create_thread(thread_start_t start, void *arg, thread_priority prioriy = thread_priority::normal,
    create_thread_flags flags = create_thread_flags::none, int32_t stack_size = DEFAULT_STACK_SIZE);

[[noreturn]] void exit_thread(uint32_t exit_code);

result<handle_t, error_code> create_mutex(uint32_t spin_count = DEFAULT_MUTEX_SPIN_COUNT, bool recursive = false) noexcept;
result<void, error_code> release_mutex(handle_t handle) noexcept;

result<void, error_code> wait(handle_t handle) noexcept;
}
