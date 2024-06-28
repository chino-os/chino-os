// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "page_table_2.h"

namespace chino::os::kernel::mm {
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

    /** @brief Take one level-2 page.
     *  @return The index of set bit.
     */
    result<size_t> take(page_table_2 &pt2) noexcept {
        // 1. Reserve one page
        while (true) {
            auto value = value_.load(std::memory_order_relaxed);
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

    /** @brief Take the whole level-1 page entry.
     */
    result<void> take_huge() noexcept {
        auto expected = initial_value; // Expect untaken
        return value_.compare_exchange_strong(expected, 1) ? ok() : err(error_code::out_of_memory);
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
  public:
    inline static constexpr size_t max_entries = 32;

    /** @brief Take one page.
     *  @return The index of set bit.
     */
    result<size_t> take(page_table_2 *pt2_bucket_base) noexcept {
        for (size_t pt1_index = 0; pt1_index < entries_.size(); pt1_index++) {
            auto index_in_pt1 = entries_[pt1_index].take(pt2_bucket_base[pt1_index]);
            if (index_in_pt1.is_ok()) {
                return ok(pt1_index * page_table_2::max_pages + index_in_pt1.unwrap());
            }
        }
        return err(error_code::out_of_memory);
    }

    /** @brief Take the whole level-1 page entry.
     *  @return The index of set bit.
     */
    result<size_t> take_huge() noexcept {
        for (size_t pt1_index = 0; pt1_index < entries_.size(); pt1_index++) {
            if (entries_[pt1_index].take_huge().is_ok()) {
                return ok(pt1_index * page_table_2::max_pages);
            }
        }
        return err(error_code::out_of_memory);
    }

  private:
    std::array<page_table_1_entry, max_entries> entries_;
};

static_assert(sizeof(page_table_1) == hal::cacheline_size);
} // namespace chino::os::kernel::mm
