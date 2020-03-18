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
#include <chino/threading.h>
#include <cstdint>
#include <gsl/gsl-lite.hpp>

namespace chino::arch
{
struct win32_thread_context
{
};

using thread_context_t = win32_thread_context;

struct win32_arch
{
    static uint32_t current_processor() noexcept { return 0; }
    static void yield_processor() noexcept;
    static uintptr_t disable_irq() noexcept;
    static void restore_irq(uintptr_t state) noexcept;
    static void init_thread_context(thread_context_t &context, gsl::span<uintptr_t> stack, threading::thread_start_t start, void *arg) noexcept;
};

using arch_t = win32_arch;
}
