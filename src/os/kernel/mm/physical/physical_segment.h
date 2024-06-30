// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "chino/error.h"
#include "chino/os/kernel/hal/cpu/cpu.h"
#include "chino/os/kernel/kernel.h"
#include "chino/units.h"
#include "page_table_0.h"
#include <cstddef>
#include <utility>

namespace chino::os::kernel::mm {
struct rent_result {
    size_t pt01_index;
    size_t free_pages;
};

class physical_segment {
  public:
    static uint32_t segments_count;
    static physical_segment &segment(size_t index) noexcept;

  public:
    constexpr physical_segment(page_table_0 *pt0_base, page_table_1 *pt1_base, page_table_2 *pt2_base) noexcept
        : physical_address_base_(0), total_pages_(0), pt0_base_(pt0_base), pt1_base_(pt1_base), pt2_base_(pt2_base) {}

    uintptr_t physical_address_base() const noexcept { return physical_address_base_; }
    page_table_0 &pt0(size_t index) noexcept { return pt0_base_[index]; }
    page_table_1 &pt1(size_t index) noexcept { return pt1_base_[index]; }
    page_table_2 &pt2(size_t index) noexcept { return pt2_base_[index]; }

    void initialize(uintptr_t physical_address, size_t size_bytes) noexcept;

    result<rent_result> rent() noexcept;
    void return_(const rent_result &rent) noexcept { return pt0(rent.pt01_index).return_(rent.free_pages); }

  private:
    size_t pt2_count() const noexcept { return ceil_div(total_pages_, page_table_2::max_pages); }
    size_t pt01_count() const noexcept { return ceil_div(pt2_count(), page_table_1::max_entries); }

  private:
    uintptr_t physical_address_base_;
    size_t total_pages_;
    page_table_0 *pt0_base_;
    page_table_1 *pt1_base_;
    page_table_2 *pt2_base_;
};

template <size_t MaxSizeBytes> class inline_physical_segment : public physical_segment {
  public:
    inline static constexpr size_t max_pages = MaxSizeBytes / hal::cpu_t::min_page_size;
    inline static constexpr size_t max_pt2_count = ceil_div(max_pages, page_table_2::max_pages);
    inline static constexpr size_t max_pt01_count = ceil_div(max_pt2_count, page_table_1::max_entries);

  public:
    constexpr inline_physical_segment() noexcept : physical_segment(pt0s_, pt1s_, pt2s_) {}

  private:
    alignas(hal::cacheline_size) page_table_0 pt0s_[max_pt01_count];
    alignas(hal::cacheline_size) page_table_1 pt1s_[max_pt01_count];
    alignas(hal::cacheline_size) page_table_2 pt2s_[max_pt2_count];
};
} // namespace chino::os::kernel::mm
