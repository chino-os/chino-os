// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "chino/os/kernel/hal/cpu/cpu.h"
#include "physical_segment.h"

namespace chino::os::kernel::mm {
class alignas(hal::cacheline_size) cpu_physical_allocator {
  public:
    constexpr cpu_physical_allocator() noexcept : segment_index_(0), pt01_index_(0) {}

    void initialize() noexcept;
    result<uintptr_t> allocate_page() noexcept;

  private:
    physical_segment &current_segment() noexcept { return physical_segment::segment(segment_index_); }
    result<void> rent_new_pt0() noexcept;
    void return_current_pt0() noexcept;

  private:
    uint32_t segment_index_;
    uint32_t pt01_index_;
    std::atomic<uint32_t> free_pages_;
    std::atomic<uint32_t> encoded_last_frees_;
};

static_assert(sizeof(cpu_physical_allocator) == hal::cacheline_size);
} // namespace chino::os::kernel::mm
