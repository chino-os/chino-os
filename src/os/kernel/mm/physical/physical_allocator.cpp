// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "physical_allocator.h"
#include "cpu_physical_allocator.h"
#include "physical_segment.h"
#include <chino/os/kernel/hal/chip.h>

using namespace chino;
using namespace chino::os::kernel;
using namespace chino::os::kernel::mm;

namespace {
constinit std::array<cpu_physical_allocator, hal::chip_t::cpus_count> cpu_allocators_;
} // namespace

void physical_allocator::initialize_phase0(const boot_options &options) noexcept {
    physical_segment::segments_count = (uint32_t)options.memory_descs.size();
    for (size_t i = 0; i < options.memory_descs.size(); i++) {
        auto &desc = options.memory_descs[i];
        auto &segment = physical_segment::segment(i);
        segment.initialize(desc.physical_address, desc.size_bytes);
    }
}

result<uintptr_t> physical_allocator ::allocate_page() noexcept {
    return cpu_allocators_[hal::arch_t::current_cpu_id()].allocate_page();
}
