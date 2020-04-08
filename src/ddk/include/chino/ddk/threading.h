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
#include <atomic>
#include <chino/error.h>
#include <chino/result.h>
#include <string_view>

namespace chino::threading
{
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

class irq_spinlock
{
public:
    constexpr irq_spinlock() noexcept
        : taken_(), irq_state_(0) {}

    irq_spinlock(sched_spinlock &) = delete;
    irq_spinlock &operator=(sched_spinlock &) = delete;

    bool try_lock() noexcept;
    void lock() noexcept;
    void unlock() noexcept;

private:
    std::atomic_flag taken_;
    uintptr_t irq_state_;
};

class kmutex
{
public:

};
}
