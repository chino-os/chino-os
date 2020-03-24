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
#include <chino/object.h>
#include <chino/threading.h>
#include <chino/utility.h>
#include <memory>
#include <mutex>

namespace chino::ob
{
struct handle_entry
{
    object_header *ob;
    access_mask granted_access;
};

namespace details
{
    class handle_segment
    {
        static constexpr size_t EntrySize = sizeof(handle_entry);

        struct entry_header
        {
            entry_header *next;
            uint32_t used;
        };

    public:
        handle_segment *next;

        static result<handle_segment *, error_code> init(size_t entries);

        result<std::pair<handle_entry *, size_t>, error_code> allocate() noexcept
        {
            std::unique_lock lock(lock_);
            if (free_head_)
            {
                auto item = reinterpret_cast<handle_entry *>(free_head_);
                free_head_ = free_head_->next;
                return ok(std::make_pair(item, index(item)));
            }
            else
            {
                return err(error_code::out_of_memory);
            }
        }

        void deallocate(handle_entry *ptr) noexcept
        {
            std::unique_lock lock(lock_);
            auto head = reinterpret_cast<entry_header *>(ptr);
            head->next = free_head_;
            head->used = 0;
            free_head_ = head;
        }

        result<handle_entry *, error_code> at(size_t index) noexcept
        {
            auto &header = entry_at(index);
            if (!header.used)
                return err(error_code::invalid_argument);
            return ok(reinterpret_cast<handle_entry *>(&header));
        }

        bool owns(handle_entry *ptr) noexcept
        {
            return ptr >= body_ && ptr < end();
        }

    private:
        handle_segment(size_t entries) noexcept
            : entries_(entries), next(nullptr), free_head_(reinterpret_cast<entry_header *>(body_))
        {
            // init entry headers
            entry_header *next = nullptr;
            entry_header *cnt = &entry_at(entries - 1);
            while (cnt >= reinterpret_cast<entry_header *>(body_))
            {
                cnt->next = next;
                cnt->used = 0;
                next = cnt;
                cnt = reinterpret_cast<entry_header *>(reinterpret_cast<uintptr_t>(cnt) - EntrySize);
            }
        }

        void *end() noexcept
        {
            return &entry_at(entries_);
        }

        entry_header &entry_at(size_t i) noexcept
        {
            return reinterpret_cast<entry_header &>(body_[i]);
        }

        size_t index(handle_entry *ptr) const noexcept
        {
            return ptr - body_;
        }

    private:
        size_t entries_;
        entry_header *free_head_;
        threading::sched_spinlock lock_;
        handle_entry body_[0];
    };
}

class handle_table
{
public:
    using segment_t = details::handle_segment;

    constexpr handle_table(size_t entries_per_segment) noexcept
        : head_(nullptr), entries_per_segment_(entries_per_segment)
    {
    }

    template <class... Args>
    result<std::pair<handle_entry *, size_t>, error_code> alloc(Args &&... args) noexcept
    {
        try_var(obj, allocate());
        new (obj.first) handle_entry(std::forward<Args>(args)...);
        return ok(obj);
    }

    void free(handle_entry *ptr) noexcept
    {
        if (ptr)
        {
            std::destroy_at(ptr);
            deallocate(ptr);
        }
    }

    result<handle_entry *, error_code> at(size_t index) noexcept
    {
        segment_t *cnt = head_;
        if (!cnt)
            return err(error_code::invalid_argument);

        while (index >= entries_per_segment_)
        {
            cnt = cnt->next;
            index -= entries_per_segment_;

            if (!cnt)
                return err(error_code::invalid_argument);
        }

        return cnt->at(index);
    }

private:
    result<std::pair<handle_entry *, size_t>, error_code> allocate() noexcept
    {
        // 1. Try allocate from existing segments
        {
            std::unique_lock lock(lock_);
            segment_t *cnt = head_;
            size_t index_base = 0;
            while (cnt)
            {
                auto ret = cnt->allocate();
                if (ret.is_ok())
                {
                    auto pair = ret.unwrap();
                    return ok(std::make_pair(pair.first, index_base + pair.second));
                }

                cnt = cnt->next;
                index_base += entries_per_segment_;
            }
        }

        // 2. Create new segment
        {
            try_var(seg, segment_t::init(entries_per_segment_));

            std::unique_lock lock(lock_);
            seg->next = head_;
            head_ = seg;
            return seg->allocate();
        }
    }

    void deallocate(handle_entry *ptr) noexcept
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
    size_t entries_per_segment_;
    segment_t *head_;
    threading::sched_spinlock lock_;
};
}
