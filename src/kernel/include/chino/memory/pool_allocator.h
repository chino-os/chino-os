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
#include <chino/utility.h>
#include <memory>

namespace chino::memory
{
namespace details
{
    struct entry_header
    {
        entry_header *next;
    };

    template <size_t EntrySize>
    class paged_pool_segment
    {
    public:
        paged_pool_segment *next;

        static result<paged_pool_segment *, error_code> init(threading::kprocess &process, size_t min_entries)
        {
            auto pages = ceil_div(sizeof(paged_pool_segment) + EntrySize * min_entries, PAGE_SIZE);
            try_var(base, allocate_pages(process, pages));
            new (base) paged_pool_segment(pages);
            return reinterpret_cast<paged_pool_segment *>(base);
        }

        result<void *, error_code> allocate() noexcept
        {
        }

    private:
        paged_pool_segment(size_t pages) noexcept
            : entries_count_((pages * PAGE_SIZE - sizeof(paged_pool_segment)) / EntrySize), next(nullptr)
        {
            // init entry headers
            entry_header *next = nullptr;
            entry_header *cnt = &entry_at(entries_count_ - 1);
            while (cnt >= reinterpret_cast<entry_header *>(body_))
            {
                cnt->next = next;
                next = cnt;
                cnt = reinterpret_cast<entry_header *>(reinterpret_cast<uintptr_t>(cnt) - EntrySize);
            }
        }

        entry_header &entry_at(size_t i) noexcept
        {
            return *reinterpret_cast<entry_header *>(body_ + i * EntrySize);
        }

    private:
        size_t entries_count_;
        uint8_t body_[0];
    };
}

template <class T>
class paged_pool_allocator
{
    static constexpr size_t ENTRY_SIZE = std::max(sizeof(details::entry_header), sizeof(T));

public:
    using segment_t = details::paged_pool_segment<ENTRY_SIZE>;

    constexpr paged_pool_allocator() noexcept
        : head_(nullptr)
    {
    }

private:
    segment_t *head_;
};
}
