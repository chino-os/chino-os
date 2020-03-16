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
#include <chino/utility.h>
#include <utility>

namespace chino::memory
{
class bitmap_allocator
{
public:
    bitmap_allocator *next = nullptr;

    bitmap_allocator(uint8_t *base, size_t pages_count)
        : base_(base), pages_count_(pages_count)
    {
        std::fill_n(storage_, elements(), uintptr_t(0));
    }

private:
    size_t elements() const noexcept
    {
        return ceil_div(pages_count_, CHAR_BIT * sizeof(uintptr_t));
    }

private:
    uint8_t *base_;
    size_t pages_count_;
    threading::sched_spinlock lock_;
    uintptr_t storage_[0];
};
}
