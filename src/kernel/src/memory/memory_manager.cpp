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
#include <chino/memory/bitmap_allocator.h>
#include <chino/memory/memory_manager.h>
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

std::atomic<size_t> avail_pages_;
bitmap_allocator *phy_mem_bit_allocator_;
static_object<kprocess> kernel_process_;
static_object<kthread> kernel_system_thread_;
}

kprocess &threading::kernel_process() noexcept
{
    return kernel_process_.body;
}

result<void, error_code> kernel::memory_manager_init(const physical_memory_desc &desc)
{
    avail_pages_ = desc.pages_count;

    // 1. Calc initial mem usage
    size_t phy_mem_bit_alloc_bytes = std::accumulate(desc.runs, desc.runs + desc.runs_count, size_t(0),
        [](size_t init, const physical_memory_run &run) {
            return init + sizeof(bitmap_allocator) + ceil_div(run.count, CHAR_BIT * sizeof(uintptr_t)) * sizeof(uintptr_t);
        });
    auto kernel_system_stack_bytes = KERNEL_STACK_SIZE;

    return ok();
}
