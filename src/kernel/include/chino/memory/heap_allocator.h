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
#include <board.h>
#include <cassert>
#include <chino/ddk/ps.h>
#include <chino/ddk/utility.h>
#include <chino/memory/memory_manager.h>
#include <chino_config.h>
#include <cstring>
#include <mutex>

namespace chino::memory
{
class heap_allocator
{
    struct free_heap_node
    {
        size_t count;
        free_heap_node *next;

        void *end() noexcept
        {
            return reinterpret_cast<uint8_t *>(this) + count;
        }
    };

    struct alloc_header
    {
        size_t count;
        uint8_t body[0];

        size_t content_len() const noexcept { return count - sizeof(alloc_header); }

        void *end() noexcept
        {
            return reinterpret_cast<uint8_t *>(this) + count;
        }
    };

public:
    constexpr heap_allocator()
        : avail_bytes_(0) {}

    result<void *, error_code> allocate(size_t bytes) noexcept
    {
        auto required_bytes = align(bytes, arch::arch_t::ALLOCATE_ALIGNMENT) + sizeof(alloc_header);
        {
            std::unique_lock lock(lock_);
            sanity_check();
        }
        // 1. Allocate from free nodes
        if (avail_bytes_ >= bytes)
        {
            std::unique_lock lock(lock_);
            sanity_check();
            free_heap_node *prev = nullptr;
            free_heap_node *cnt = head_;
            while (cnt)
            {
                auto avail = cnt->count;
                bool found = false;

                // 1.1. Split
                if (avail >= required_bytes + sizeof(free_heap_node))
                {
                    cnt->count -= required_bytes;
                    auto header = reinterpret_cast<alloc_header *>(cnt->end());
                    header->count = required_bytes;
                    avail_bytes_ -= required_bytes;
                    sanity_check();
                    return ok((void *)header->body);
                }
                // 1.2. Remove
                else if (avail >= required_bytes)
                {
                    if (prev)
                        prev->next = cnt->next;
                    else
                        head_ = cnt->next;
                    auto count = cnt->count;
                    auto header = reinterpret_cast<alloc_header *>(cnt);
                    header->count = count;
                    avail_bytes_ -= count;
                    sanity_check();
                    return ok((void *)header->body);
                }

                prev = cnt;
                cnt = cnt->next;
            }
        }

        // 2. Allocate from new page
        {
            auto page_num = ceil_div(required_bytes, PAGE_SIZE);
            try_var(pages, allocate_pages(owner(), page_num));
            // 2.1. Init allocate hader
            auto header = reinterpret_cast<alloc_header *>(pages);
            header->count = required_bytes;
            // 2.2. Rest free space
            auto rest_free = page_num * PAGE_SIZE - required_bytes;
            if (rest_free)
                insert_free_node(reinterpret_cast<uint8_t *>(pages) + required_bytes, rest_free);
            return ok((void *)header->body);
        }

        return err(error_code::out_of_memory);
    }

    void free(void *ptr) noexcept
    {
        if (!ptr)
            return;

        auto h = header(ptr);
        insert_free_node(h, h->count);
    }

    result<void *, error_code> realloc(void *ptr, size_t bytes) noexcept
    {
        if (!bytes)
        {
            free(ptr);
            return ok<void *>(nullptr);
        }

        if (!ptr)
            return allocate(bytes);

        auto h = header(ptr);
        if (bytes <= h->content_len())
        {
            return ok(ptr);
        }
        else
        {
            try_var(new_block, allocate(bytes));
            {
                std::unique_lock lock(lock_);
                sanity_check();
                std::memcpy(new_block, ptr, std::min(bytes, h->content_len()));
                sanity_check();
            }
            free(ptr);
            return ok(new_block);
        }
    }

private:
    alloc_header *header(void *ptr)
    {
        return reinterpret_cast<alloc_header *>(reinterpret_cast<uint8_t *>(ptr) - sizeof(alloc_header));
    }

    void insert_free_node(void *base, size_t count)
    {
        assert(count >= sizeof(free_heap_node));
        std::unique_lock lock(lock_);
        sanity_check();
        avail_bytes_ += count;

        // 1. No free
        if (!head_)
        {
            auto node = reinterpret_cast<free_heap_node *>(base);
            node->count = count;
            node->next = nullptr;
            head_ = node;
            sanity_check();
        }
        else
        {
            free_heap_node *prev = nullptr;
            free_heap_node *cnt = head_;
            // 2.1. Skip previous
            while (cnt && base > cnt)
            {
                prev = cnt;
                cnt = cnt->next;
            }

            auto node = reinterpret_cast<free_heap_node *>(base);
            node->count = count;
            node->next = cnt;
            if (prev)
                prev->next = node;
            else
                head_ = node;
            sanity_check();
            merge_node(prev, node, cnt);
        }
    }

    void merge_node(free_heap_node *prev, free_heap_node *cnt, free_heap_node *next) noexcept
    {
        if (cnt->end() == next)
        {
            cnt->count += next->count;
            cnt->next = next->next;
        }

        if (prev && prev->end() == cnt)
        {
            prev->count += cnt->count;
            prev->next = cnt->next;
            try_free_pages(prev);
        }
        else
        {
            try_free_pages(cnt);
        }

        sanity_check();
    }

    void try_free_pages(free_heap_node *node) noexcept
    {
        if (node->count >= PAGE_SIZE)
        {
            auto aligned_low = align(uintptr_t(node), PAGE_SIZE);
            auto aligned_high = align_down(uintptr_t(node->end()), PAGE_SIZE);

            if (aligned_high > aligned_low)
            {
                intmax_t pages = (aligned_high - aligned_low) / PAGE_SIZE;
                intmax_t free_before = aligned_low - uintptr_t(node);
                intmax_t free_after = uintptr_t(node->end()) - aligned_high;

                if (free_after != 0 && free_after < sizeof(free_heap_node))
                {
                    pages--;
                    free_after += PAGE_SIZE;
                }

                if (free_before != 0 && free_before < sizeof(free_heap_node))
                {
                    pages--;
                    free_before += PAGE_SIZE;
                }

                // No pages to be free
                if (pages <= 0)
                    return;

                // Add free after node
                if (free_after)
                {
                    auto new_node = reinterpret_cast<free_heap_node *>(uintptr_t(node->end()) - free_after);
                    new_node->count = free_after;
                    new_node->next = node->next;
                    node->next = new_node;
                }

                node->count -= pages * PAGE_SIZE;

                // Add free before node
                if (free_before)
                {
                    node->count = free_before;
                }
                else
                {
                    // Find prev node
                    free_heap_node *prev = nullptr, *cnt = head_;
                    while (cnt != node)
                    {
                        prev = cnt;
                        cnt = cnt->next;
                    }

                    if (prev)
                        prev->next = node->next;
                    else
                        head_ = node->next;
                }

                // Free pages
                free_pages(owner(), reinterpret_cast<uint8_t *>(node) + free_before, pages);
                avail_bytes_ -= pages * PAGE_SIZE;
            }
        }
    }

    threading::kprocess &owner() noexcept;

private:
    void sanity_check()
    {
        size_t avail = 0;
        free_heap_node *cnt = head_;
        // 2.1. Skip previous
        while (cnt)
        {
            avail += cnt->count;
            cnt = cnt->next;
        }

        assert(avail_bytes_ == avail);
        assert(avail_bytes_ < 102400);
    }

private:
    std::atomic<size_t> avail_bytes_;
    threading::sched_spinlock lock_;
    free_heap_node *head_ = nullptr;
};
}
