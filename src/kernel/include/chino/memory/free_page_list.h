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
class free_page_list
{
    struct free_page_node
    {
        size_t count;
        free_page_node *next;

        void *end() noexcept
        {
            return reinterpret_cast<uint8_t *>(this) + count * PAGE_SIZE;
        }
    };

    static_assert(sizeof(free_page_node) <= PAGE_SIZE);

public:
    void init(const kernel::physical_memory_desc &desc) noexcept
    {
        avail_pages_ = desc.pages_count;
        free_page_node *prev = nullptr;
        for (size_t i = 0; i < desc.runs_count; i++)
        {
            auto &run = desc.runs[i];
            auto node = reinterpret_cast<free_page_node *>(run.base);
            node->count = run.count;
            if (prev)
                prev->next = node;
            else
                head_ = node;
            prev = node;
        }
    }

    result<void *, error_code> allocate(size_t pages) noexcept
    {
        if (avail_pages_ < pages)
            return err(error_code::out_of_memory);

        std::unique_lock lock(lock_);
        free_page_node *prev = nullptr;
        free_page_node *cnt = head_;
        while (cnt)
        {
            auto avail = cnt->count;
            bool found = false;

            // 1. Split
            if (avail > pages)
            {
                cnt->count -= pages;
                found = true;
            }
            // 2. Remove
            else if (avail == pages)
            {
                if (prev)
                    prev->next = cnt->next;
                else
                    head_ = cnt->next;
                found = true;
            }

            if (found)
            {
                return ok((void *)(reinterpret_cast<uint8_t *>(cnt) + pages * PAGE_SIZE));
            }
            else
            {
                prev = cnt;
                cnt = cnt->next;
            }
        }

        return err(error_code::out_of_memory);
    }

    void free(void *base, size_t pages) noexcept
    {
        std::unique_lock lock(lock_);

        // 1. No free
        if (!head_)
        {
            auto node = reinterpret_cast<free_page_node *>(base);
            node->count = pages;
            node->next = nullptr;
            head_ = node;
        }
        else
        {
            free_page_node *prev = nullptr;
            free_page_node *cnt = head_;
            // 2.1. Skip previous
            while (base < cnt)
            {
                prev = cnt;
                cnt = cnt->next;
            }

            auto node = reinterpret_cast<free_page_node *>(base);
            node->count = pages;
            node->next = cnt;
            if (prev)
                prev->next = node;
            else
                head_ = node;
            merge_node(prev, node, cnt);
        }
    }

private:
    void merge_node(free_page_node *prev, free_page_node *cnt, free_page_node *next) noexcept
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
        }
    }

private:
    std::atomic<size_t> avail_pages_;
    threading::sched_spinlock lock_;
    free_page_node *head_ = nullptr;
};
}
