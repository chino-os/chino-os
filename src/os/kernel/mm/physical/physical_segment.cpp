// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "physical_segment.h"
#include <chino/os/hal/chip.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::mm;

namespace {
template <size_t SegmentIndex>
struct physical_segment_impl : public inline_physical_segment<hal::chip_t::max_memory_segments_sizes[SegmentIndex]> {};

template <class TSegmentIndexes> class physical_segments_impl;
template <size_t... SegmentIndexes>
class physical_segments_impl<std::integer_sequence<size_t, SegmentIndexes...>>
    : public physical_segment_impl<SegmentIndexes>... {
  public:
    physical_segment &segment(size_t index) noexcept {
        physical_segment *segments[] = {static_cast<physical_segment_impl<SegmentIndexes> *>(this)...};
        return *segments[index];
    }
};

using physical_segments_t =
    physical_segments_impl<std::make_index_sequence<hal::chip_t::max_memory_segments_sizes.size()>>;
constinit physical_segments_t physical_segments_;
} // namespace

uint32_t physical_segment::segments_count;

physical_segment &physical_segment::segment(size_t index) noexcept { return physical_segments_.segment(index); }

void physical_segment::initialize(uintptr_t physical_address, size_t size_bytes) noexcept {
    physical_address_base_ = physical_address;
    total_pages_ = size_bytes / hal::arch_t::min_page_size;
}

result<rent_result> physical_segment::rent() noexcept {
    for (size_t i = 0; i < pt01_count(); i++) {
        auto pages = pt0(i).rent();
        if (pages.is_ok()) {
            return ok(rent_result{(uint32_t)i, pages.unwrap()});
        }
    }

    return err(error_code::out_of_memory);
}
