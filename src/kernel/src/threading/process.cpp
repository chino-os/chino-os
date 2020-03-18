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
#include <chino/kernel.h>
#include <chino/threading/process.h>

using namespace chino;
using namespace chino::arch;
using namespace chino::ob;
using namespace chino::threading;
using namespace chino::memory;

namespace
{
static uintptr_t kernel_sys_thread_stack_[KERNEL_STACK_SIZE / sizeof(uintptr_t)];

static_object<kprocess> kernel_process_;
static_object<kthread> kernel_system_thread_;
}

kprocess &threading::kernel_process() noexcept
{
    return kernel_process_.body;
}

result<void, error_code> threading::kernel_process_init() noexcept
{
    kernel_system_thread_.body.init_stack(kernel_sys_thread_stack_, kernel::kernel_system_thread_main, nullptr);
    kernel_process_.body.attach_new_thread(kernel_system_thread_.body);

    return ok();
}

void kprocess::attach_new_thread(kthread &thread) noexcept
{
    thread.owner_ = this;
    threads_list_.add_last(&thread.process_threads_entry_);
}

void kthread::init_stack(gsl::span<uintptr_t> stack, threading::thread_start_t start, void *arg) noexcept
{
    stack_ = stack;
    arch_t::init_thread_context(context_, stack, start, arg);
}
