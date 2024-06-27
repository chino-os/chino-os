// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "page_table_2.h"

namespace chino::os::kernel::mm {
static_assert(hal::cpu_t::min_page_size == 4 * KiB, "Only support 4KiB base page");

/** @brief Level-1 page table entry.
 *
 *  Counts for untaken level-2 pages and marks whether the whole level-1 page entry is taken.
 */
class page_table_1_entry {
  public:
    inline static constexpr size_t max_pages = page_table_2::max_pages;

  private:
    inline static constexpr uint16_t initial_value = max_pages << 1;

  public:
    constexpr page_table_1_entry() noexcept : value_(initial_value) {}

    /** @brief Take the whole level-1 page entry.
     */
    result<void> take_all() noexcept {
        uint16_t expected = initial_value; // Expect untaken
        return value_.compare_exchange_strong(expected, 1) ? ok() : err(error_code::out_of_memory);
    }

    /** @brief Take one level-2 page.
     *  @return The index of set bit.
     */
    result<size_t> take(page_table_2 &pt2) noexcept {
        // 1. Reserve one page
        while (true) {
            uint16_t value = value_.load(std::memory_order_relaxed);
            if (value == 0 || (value & 1)) {
                // 1.1 No free pages
                return err(error_code::out_of_memory);
            } else {
                // 1.2 Minus 1
                auto desired = value - (1 << 1);
                if (value_.compare_exchange_strong(value, desired)) {
                    break;
                }
            }
        }

        // 2. Take level-2 page, shouldn't fail
        return ok(pt2.set_first_untaken().unwrap());
    }

    size_t free_pages() const noexcept {
        auto value = value_.load(std::memory_order_relaxed);
        return (value & 1) ? 0 // Huge page
                           : value >> 1;
    }

  private:
    std::atomic<uint16_t> value_;
};

/** @brief Level-1 page table.
 *
 *  Consist of 32 level-1 page table entries.
 */
class page_table_1 {
  private:
    std::array<page_table_1_entry, 32> entries_;
};

static_assert(sizeof(page_table_1) == hal::cacheline_size);
} // namespace chino::os::kernel::mm
