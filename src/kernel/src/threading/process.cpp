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
#include <chino/ddk/kernel.h>
#include <chino/threading/process.h>
#include <chino/threading/scheduler.h>

using namespace chino;
using namespace chino::arch;
using namespace chino::ob;
using namespace chino::threading;
using namespace chino::memory;

const object_type wellknown_types::process { .ops = {} };
const object_type wellknown_types::thread { .ops = {} };

namespace
{
static_object<kprocess> kernel_process_(wellknown_types::process);
static_object<kthread> kernel_system_thread_(wellknown_types::thread);

alignas(STACK_ALIGNMENT) uintptr_t kernel_sys_thread_stack_[KERNEL_STACK_SIZE / sizeof(uintptr_t)];
}

[[noreturn]] static void user_thread_thunk(thread_start_t start, void *arg) noexcept;

kprocess &kernel::kernel_process() noexcept
{
    return kernel_process_.body;
}

result<void, error_code> kernel::kernel_process_init() noexcept
{
    kernel_system_thread_.body.priority_ = thread_priority::normal;
    kernel_system_thread_.body.init_stack(kernel_sys_thread_stack_, kernel::kernel_system_thread_main, nullptr);
    kernel_process_.body.attach_new_thread(kernel_system_thread_.body);
    current_sched().add_to_ready_list(kernel_system_thread_.body);

    return ok();
}

void kprocess::attach_new_thread(kthread &thread) noexcept
{
    thread.owner_ = this;
    threads_list_.add_last(&thread.process_threads_entry_);
}

void kprocess::detach_thread(kthread &thread) noexcept
{
    assert(thread.owner_ == this);
    threads_list_.remove(&thread.process_threads_entry_);
}

void kthread::init_stack(gsl::span<uintptr_t> stack, thread_start_t start, void *arg) noexcept
{
    stack_ = stack;
    arch_t::init_thread_context(context_, stack, (kernel::thread_thunk_t)user_thread_thunk, start, arg);
}

void user_thread_thunk(thread_start_t start, void *arg) noexcept
{
    auto exit_code = start(arg);
    exit_thread(exit_code);
}

result<void, error_code> process_open(void *object, access_state &access) noexcept
{
    return ok();
}

result<void, error_code> thread_open(void *object, access_state &access) noexcept
{
    return ok();
}
