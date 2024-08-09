// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "page_table_1.h"
#include <atomic>

namespace chino::os::kernel::mm {
/** @brief Level-1 page table.
 *
 *  Counts for untaken level-1 pages and marks whether the whole level-0 page entry is reserved.
 */
class page_table_0 {
  public:
    inline static constexpr size_t max_pages = page_table_1_entry::max_pages * page_table_1::max_entries;

  private:
    inline static constexpr uint16_t initial_value = max_pages << 1;

  public:
    constexpr page_table_0() noexcept : value_(initial_value) {}

    /** @brief Rent the level-0 page.
     *  @return The count of free level-2 pages.
     */
    result<uint16_t> rent() noexcept {
        while (true) {
            auto value = value_.load(std::memory_order_relaxed);
            if (value & 1) {
                // 1.1 Already rented
                return err(error_code::out_of_memory);
            } else {
                // 1.2 Set rent bit
                if (value_.compare_exchange_strong(value, 1)) {
                    return ok(value >> 1);
                }
            }
        }
    }

    /** @brief Return the rented level-0 page.
     */
    void return_(uint16_t free_pages) noexcept {
        auto add_value = (free_pages << 1) - 1; // Unset rent bit
        value_.fetch_add(add_value, std::memory_order_relaxed);
    }

    size_t free_pages() const noexcept { return value_.load(std::memory_order_relaxed) >> 1; }

  private:
    std::atomic<uint16_t> value_;
};
} // namespace chino::os::kernel::mm
