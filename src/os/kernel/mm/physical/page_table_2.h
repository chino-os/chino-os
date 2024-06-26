// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <atomic>
#include <chino/os/kernel/hal/cpu/cpu.h>
#include <chino/os/kernel/kernel.h>
#include <chino/result.h>
#include <numeric>

namespace chino::os::kernel::mm {
static_assert(hal::cpu_t::min_page_size == 4 * KiB, "Only support 4KiB base page");

/** @brief Level-2 page table.
 *
 *  Each bit represents a base page (4KiB), so the whole l2 table represents 4*512=2MiB.
 */
class page_table_2 {
  private:
    inline static constexpr size_t atomics_count = 512 / (sizeof(uint64_t) * 8);

  public:
    constexpr page_table_2() noexcept : taken_{} {}

    /** @brief Atomic search for the first unset bit and set it to 1. */
    result<void> set_any() {

    }

  private:
    std::atomic<uint64_t> taken_[atomics_count];
};
} // namespace chino::os::kernel::mm
