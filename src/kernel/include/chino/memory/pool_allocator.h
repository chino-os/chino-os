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
#include <chino/threading.h>
#include <chino/utility.h>
#include <memory>
#include <mutex>

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
            std::unique_lock lock(lock_);
            if (free_head_)
            {
                void *item = free_head_;
                free_head_ = free_head_->next;
                return ok(item);
            }
            else
            {
                return err(error_code::unavailable);
            }
        }

        void deallocate(void *ptr) noexcept
        {
            std::unique_lock lock(lock_);
            auto head = reinterpret_cast<entry_header *>(ptr);
            head->next = free_head_;
            free_head_ = head;
        }

        bool owns(void *ptr) const noexcept
        {
            return ptr >= body_ && ptr < end();
        }

    private:
        paged_pool_segment(size_t pages) noexcept
            : entries_count_((pages * PAGE_SIZE - sizeof(paged_pool_segment)) / EntrySize), next(nullptr), free_head_(reinterpret_cast<entry_header *>(body))
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

        void *end() noexcept
        {
            return &entry_at(entries_count_);
        }

        entry_header &entry_at(size_t i) noexcept
        {
            return *reinterpret_cast<entry_header *>(body_ + i * EntrySize);
        }

    private:
        size_t entries_count_;
        entry_header *free_head_;
        threading::sched_spinlock lock_;
        uint8_t body_[0];
    };
}

template <class T>
class paged_pool_allocator
{
    static constexpr size_t ENTRY_SIZE = std::max(sizeof(details::entry_header), sizeof(T));

public:
    using segment_t = details::paged_pool_segment<ENTRY_SIZE>;

    constexpr paged_pool_allocator(threading::kprocess &process, size_t min_entries_per_segment) noexcept
        : process_(process), head_(nullptr), min_entries_per_segment_(min_entries_per_segment)
    {
    }

    template <class... Args>
    result<T *, error_code> alloc(Args &&... args) noexcept
    {
        try_var(obj, allocate());
        new (obj) T(std::forward<Args>(args)...);
        return obj;
    }

    void free(T *ptr) noexcept
    {
        if (ptr)
        {
            ptr->~T();
            deallocate(ptr);
        }
    }

private:
    result<void *, error_code> allocate() noexcept
    {
        // 1. Try allocate from existing segments
        {
            std::unique_lock lock(lock_);
            segment_t *cnt = head_;
            while (cnt)
            {
                auto ret = cnt->allocate();
                if (ret.is_ok())
                    return ret;
                cnt = cnt->next;
            }
        }

        // 2. Create new segment
        {
            try_var(seg, segment_t::init(process_, min_entries_per_segment_));

            std::unique_lock lock(lock_);
            seg->next = head_;
            head_ = seg;
            return seg->allocate();
        }
    }

    void deallocate(void *ptr) noexcept
    {
        segment_t *seg = nullptr;

        // 1. Find the owner segment
        {
            std::unique_lock lock(lock_);
            segment_t *cnt = head_;
            while (cnt)
            {
                if (cnt->owns(ptr))
                {
                    seg = cnt;
                    break;
                }

                cnt = cnt->next;
            }
        }

        // 2. Deallocate
        seg->deallocate(ptr);
    }

private:
    threading::kprocess &process_;
    size_t min_entries_per_segment_;
    segment_t *head_;
    threading::sched_spinlock lock_;
};
}
