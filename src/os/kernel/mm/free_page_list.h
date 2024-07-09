// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <atomic>
#include <chino/os/hal/arch.h>
#include <chino/os/kernel/ke.h>
#include <chino/result.h>
#include <numeric>

namespace chino::os::kernel::mm {
class free_page_list {
    struct free_page_node {
        free_page_node *next;
        size_t pages;
        std::byte *end() noexcept { return reinterpret_cast<std::byte *>(this) + pages * hal::arch_t::min_page_size; }
    };

    static_assert(sizeof(free_page_node) <= hal::arch_t::min_page_size);

  public:
    constexpr free_page_list() : avail_pages_(0), head_(nullptr) {}

    size_t avail_pages() const noexcept { return avail_pages_.load(); }

    void initialize(std::span<const boot_memory_desc> descs) noexcept;

    result<std::byte *> allocate() noexcept;
    void free(void *base) noexcept;

  private:
    std::atomic<size_t> avail_pages_;
    free_page_node *head_;
};
} // namespace chino::os::kernel::mm
