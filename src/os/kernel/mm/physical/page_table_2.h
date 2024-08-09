// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <array>
#include <atomic>
#include <bit>
#include <chino/os/hal/arch.h>
#include <chino/os/kernel/ke.h>
#include <chino/result.h>
#include <numeric>

namespace chino::os::kernel::mm {
static_assert(hal::arch_t::min_page_size == 4 * KiB, "Only support 4KiB base page");

/** @brief Level-2 page table.
 *
 *  Each bit represents a base page (4KiB), so the whole l2 table represents 4*512=2MiB.
 */
class page_table_2 {
  public:
    inline static constexpr size_t max_pages = 512;
    inline static constexpr size_t row_digits = std::numeric_limits<uint64_t>::digits;
    inline static constexpr size_t atomics_count = max_pages / row_digits;

  public:
    constexpr page_table_2() noexcept : rows_{} {}

    /** @brief Atomic search for the first unset bit and set it to 1.
     *  @return The index of set bit.
     */
    result<size_t> set_first_untaken() noexcept {
        for (size_t row_index = 0; row_index < rows_.size(); row_index++) {
            auto &row = rows_[row_index];
            while (true) {
                auto row_value = row.load(std::memory_order_relaxed);
                if (row_value != std::numeric_limits<uint64_t>::max()) {
                    auto first_zero_index = (size_t)std::countr_one(row_value);
                    CHINO_ASSUME(first_zero_index > 0 && first_zero_index < row_digits);
                    auto mask = uint64_t(0b1) << first_zero_index;
                    if ((row.fetch_or(mask, std::memory_order_relaxed) & mask) == 0) {
                        // Successfully set without race condition.
                        return ok(row_index * row_digits + first_zero_index);
                    }
                } else {
                    break;
                }
            }
        }

        return err(error_code::out_of_memory);
    }

    bool taken(size_t index) const noexcept {
        const auto index2d = to_index2d(index);
        return rows_[index2d.first].load(std::memory_order_relaxed) & (uint64_t(0b1) << index2d.second);
    }

  private:
    static constexpr std::pair<size_t, size_t> to_index2d(size_t linear_index) noexcept {
        return {linear_index / row_digits, linear_index % row_digits};
    }

  private:
    std::array<std::atomic<uint64_t>, atomics_count> rows_;
};

static_assert(sizeof(page_table_2) == hal::cacheline_size);
} // namespace chino::os::kernel::mm
