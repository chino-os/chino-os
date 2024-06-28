// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "page_table_0.h"

namespace chino::os::kernel::mm {
template <size_t MaxSizeBytes> class physical_segment {
  public:
    inline static constexpr size_t max_pages = MaxSizeBytes / hal::cpu_t::min_page_size;
    inline static constexpr size_t max_pt2_count = ceil_div(max_pages, page_table_2::max_pages);
    inline static constexpr size_t max_pt1_count = ceil_div(max_pt2_count, page_table_1::max_entries);
    inline static constexpr size_t max_pt0_count = max_pt1_count;

  public:
    void initialize(uintptr_t physical_address, size_t size_bytes) {
        physical_address_base_ = physical_address;
        total_pages_ = size_bytes / hal::cpu_t::min_page_size;
    }

    result<uintptr_t> take() noexcept {

    }

  private:
    uintptr_t physical_address_base_;
    size_t total_pages_;

    alignas(hal::cacheline_size) page_table_2 pt2s_[max_pt2_count];
    alignas(hal::cacheline_size) page_table_1 pt1s_[max_pt1_count];
    alignas(hal::cacheline_size) page_table_0 pt0s_[max_pt0_count];
};
} // namespace chino::os::kernel::mm
