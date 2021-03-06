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
#include <chino/error.h>
#include <chino/result.h>
#include <gsl/gsl-lite.hpp>

namespace chino::threading
{
struct kprocess;
}

namespace chino::kernel
{
struct physical_memory_run
{
    void *base;
    size_t count;
};

struct physical_memory_desc
{
    size_t runs_count;
    size_t pages_count;
    physical_memory_run runs[1];
};

typedef void (*thread_thunk_t)(void *arg0, void *arg1);
result<void, error_code> memory_manager_init(const physical_memory_desc &desc);
result<void, error_code> kernel_main();
result<void, error_code> io_manager_init();
result<void, error_code> logging_init();
uint32_t kernel_system_thread_main(void *arg);

threading::kprocess &kernel_process() noexcept;
result<void, error_code> kernel_process_init() noexcept;

result<void *, error_code> kheap_alloc(size_t bytes) noexcept;
void kheap_free(void *ptr) noexcept;
}
