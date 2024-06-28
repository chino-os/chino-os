// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "page_table_1.h"

namespace chino::os::kernel::mm {
/** @brief Level-1 page table entry.
 *
 *  Counts for untaken level-1 pages and marks whether the whole level-0 page entry is reserved.
 */
class page_table_0_entry {
  public:
    inline static constexpr size_t max_pages = page_table_1_entry::max_pages * page_table_1::max_entries;

  private:
    inline static constexpr uint16_t initial_value = max_pages << 1;

  public:
    constexpr page_table_0_entry() noexcept : value_(initial_value) {}

    /** @brief Take one level-2 page.
     *  @return The index of set bit.
     */
    result<size_t> take(page_table_1 &pt1, page_table_2 *pt2_bucket_base) noexcept {
        // 1. Reserve one page
        while (true) {
            uint16_t value = value_.load(std::memory_order_relaxed);
            if ((value >> 1) == 0) {
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

        // 2. Take page, shouldn't fail
        return ok(pt1.take(pt2_bucket_base).unwrap());
    }

    /** @brief Take one huge level-1 page entry.
     *  @return The index of set bit.
     */
    result<size_t> take_huge(page_table_1 &pt1) noexcept {
        // 1. Reserve one huge page
        while (true) {
            uint16_t value = value_.load(std::memory_order_relaxed);
            if ((value >> 1) < page_table_1_entry::max_pages) {
                // 1.1 No free pages
                return err(error_code::out_of_memory);
            } else {
                // 1.2 Minus one huge page
                auto desired = value - (page_table_1_entry::max_pages << 1);
                if (value_.compare_exchange_strong(value, desired)) {
                    break;
                }
            }
        }

        // 2. Take page, shouldn't fail
        return ok(pt1.take_huge().unwrap());
    }

    /** @brief Reserve the level-0 page.
     *  @return The count of free level-2 pages.
     */
    result<size_t> reserve() noexcept {

    }

    size_t free_pages() const noexcept { return value_.load(std::memory_order_relaxed) >> 1; }

  private:
    std::atomic<uint16_t> value_;
};

/** @brief Level-1 page table.
 *
 *  Consist of 32 level-1 page table entries.
 */
class page_table_0 {
  private:
    std::array<page_table_1_entry, 32> entries_;
};

static_assert(sizeof(page_table_1) == hal::cacheline_size);
} // namespace chino::os::kernel::mm
