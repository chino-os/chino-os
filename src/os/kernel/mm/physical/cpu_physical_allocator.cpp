// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "cpu_physical_allocator.h"
#include "chino/error.h"
#include "page_table_1.h"
#include "physical_segment.h"
#include <atomic>
#include <chino/os/kernel/hal/chip.h>
#include <chino/os/kernel/threading.h>

using namespace chino;
using namespace chino::os::kernel;
using namespace chino::os::kernel::mm;

namespace {} // namespace

void cpu_physical_allocator::initialize() noexcept { rent_new_pt0().expect("Out of memory."); }

result<uintptr_t> cpu_physical_allocator::allocate_page() noexcept {
    while (true) {
        auto free_pages = free_pages_.load(std::memory_order_acquire);
        if (free_pages > 0) {
            // 1. Current pt0 is available
            if (free_pages_.compare_exchange_strong(free_pages, free_pages - 1, std::memory_order_relaxed)) {
                auto pfn =
                    current_segment().pt1(pt01_index_).take(current_segment().pt2_bucket_base(pt01_index_)).unwrap();
                return ok(current_segment().pfn_to_paddr(pfn));
            }
        } else {
            // 2. Rent new pt0
            ps::current_schedule_lock sched_lock;

            // 2.1 Check pt0 is empty again
            free_pages = free_pages_.load(std::memory_order_acquire);
            if (free_pages == 0) {
                auto rent_result = rent_new_pt0();
                if (rent_result.is_err()) {
                    return err(rent_result.unwrap_err());
                }
            }
        }
    }
}

result<void> cpu_physical_allocator::rent_new_pt0() noexcept {
    for (size_t i = 0; i < physical_segment::segments_count; i++) {
        auto rent_result = physical_segment::segment(i).rent();
        if (rent_result.is_ok()) {
            segment_index_ = (uint32_t)i;
            pt01_index_ = rent_result.unwrap().pt01_index;
            free_pages_.store(rent_result.unwrap().free_pages, std::memory_order_release);
            return ok();
        }
    }
    return err(error_code::out_of_memory);
}

void cpu_physical_allocator::return_current_pt0() noexcept {
    physical_segment::segment(segment_index_)
        .return_({.pt01_index = pt01_index_, .free_pages = free_pages_.load(std::memory_order_relaxed)});
}
