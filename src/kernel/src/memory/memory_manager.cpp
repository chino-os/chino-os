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
#include <chino/memory/free_page_list.h>
#include <chino/memory/memory_manager.h>
#include <chino/memory/pool_allocator.h>
#include <chino/memory/used_page_list.h>
#include <chino/threading/process.h>
#include <chino/threading/thread.h>
#include <chino/utility.h>
#include <numeric>

using namespace chino;
using namespace chino::ob;
using namespace chino::kernel;
using namespace chino::threading;
using namespace chino::memory;

namespace
{
static_object<kprocess> kernel_process_;
static_object<kthread> kernel_system_thread_;

free_page_list free_page_list_;
}

kprocess &threading::kernel_process() noexcept
{
    return kernel_process_.body;
}

result<void, error_code> kernel::memory_manager_init(const physical_memory_desc &desc)
{
    // 1. Calc free page list
    free_page_list_.init(desc);

    return ok();
}

result<void *, error_code> memory::allocate_pages(kprocess &process, uint32_t pages) noexcept
{
    // 1. Allocate from free list
    try_var(base, free_page_list_.allocate(pages));
    // 2. Mark process used list
}

used_page_node *&used_page_list::head(kprocess &process) noexcept
{
    return process.used_page_head_;
}
