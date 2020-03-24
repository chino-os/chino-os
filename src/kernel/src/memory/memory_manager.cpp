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
#include <cassert>
#include <chino/kernel.h>
#include <chino/memory/free_page_list.h>
#include <chino/memory/memory_manager.h>
#include <chino/memory/pool_allocator.h>
#include <chino/memory/used_page_bitmap.h>
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
free_page_list free_page_list_;
physical_memory_desc *phy_mem_desc_;
used_page_bitmap *used_page_;
}

result<void, error_code> kernel::memory_manager_init(const physical_memory_desc &desc)
{
    // 1. Init free page list
    free_page_list_.init(desc);
    // 2. Calc initial memory usage
    auto phy_mem_desc_bytes = sizeof(physical_memory_desc) + sizeof(physical_memory_run) * (desc.runs_count - 1);
    auto used_page_bytes = sizeof(used_page_bitmap) + sizeof(pid_t) * (desc.pages_count - 1);
    auto inital_pages_num = ceil_div(phy_mem_desc_bytes + used_page_bytes, PAGE_SIZE);
    // 3. Allocate from free list
    try_var(inital_pages, free_page_list_.allocate(inital_pages_num));
    // 4. Init vars
    phy_mem_desc_ = reinterpret_cast<decltype(phy_mem_desc_)>(inital_pages);
    std::memcpy(phy_mem_desc_, &desc, phy_mem_desc_bytes);

    used_page_ = reinterpret_cast<decltype(used_page_)>((uintptr_t)inital_pages + phy_mem_desc_bytes);
    used_page_->init(desc.pages_count);
    // 5. Mark used pages
    used_page_->mark(kernel_process(), inital_pages, inital_pages_num);
    return ok();
}

result<void *, error_code> memory::allocate_pages(kprocess &process, size_t pages) noexcept
{
    return used_page_->allocate(process, pages);
}

void memory::free_pages(threading::kprocess &process, void *base, size_t pages) noexcept
{
    return used_page_->free(process, base, pages);
}

system_memory_info chino::get_system_mem_info() noexcept
{
    auto total = phy_mem_desc_->pages_count;
    auto free = free_page_list_.pages();
    auto used = total - free;
    return { .page_size = PAGE_SIZE, .used_pages = (uint32_t)used, .free_pages = (uint32_t)free };
}

size_t used_page_bitmap::page_index(void *ptr) noexcept
{
    physical_memory_run *base_run = nullptr;
    for (size_t i = 0; i < phy_mem_desc_->runs_count; i++)
    {
        auto &run = phy_mem_desc_->runs[i];
        if (ptr >= run.base)
            base_run = &run;
    }

    assert(base_run);
    return (uintptr_t(ptr) - uintptr_t(base_run->base)) / PAGE_SIZE;
}

void used_page_bitmap::init(size_t pages) noexcept
{
    std::fill_n(state_, pages, INVALID_PID);
}

void used_page_bitmap::mark(threading::kprocess &process, void *base, size_t pages) noexcept
{
    auto index = page_index(base);
    std::unique_lock lock(lock_);
    std::fill_n(state_ + index, pages, process.pid_);
}

result<void *, error_code> used_page_bitmap::allocate(threading::kprocess &process, size_t pages) noexcept
{
    // 1. Allocate from free list
    try_var(base, free_page_list_.allocate(pages));
    // 2. Mark process used list
    mark(process, base, pages);
    return ok(base);
}

void used_page_bitmap::free(threading::kprocess &process, void *base, size_t pages) noexcept
{
    // 1. Unmark process used list
    {
        auto index = page_index(base);
        std::unique_lock lock(lock_);
        std::fill_n(state_ + index, pages, INVALID_PID);
    }

    // 2. Deallocate from free list
    free_page_list_.free(base, pages);
}
