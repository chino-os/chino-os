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
#include <algorithm>
#include <chino/memory/memory_manager.h>
#include <chino/memory/pool_allocator.h>
#include <chino/threading.h>
#include <chino/utility.h>
#include <memory>
#include <mutex>

namespace chino::memory
{
class used_page_list
{
public:
    constexpr used_page_list() noexcept
        : allocator_(16)
    {
    }

    result<void *, error_code> allocate(threading::kprocess &process, void *base, size_t pages) noexcept
    {
    }

    void free(threading::kprocess &process, void *base, size_t pages) noexcept
    {
    }

private:
    used_page_node *&head(threading::kprocess &process) noexcept;

private:
    pool_allocator<used_page_node> allocator_;
    threading::sched_spinlock lock_;
};
}
