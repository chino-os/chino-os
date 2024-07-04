// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "physical_allocator.h"
#include "physical_segment.h"
#include <chino/os/kernel/hal/chip/chip.h>

using namespace chino::os::kernel;
using namespace chino::os::kernel::mm;

namespace {} // namespace

void physical_allocator::initialize_phase0(const boot_options &options) noexcept {
    physical_segment::segments_count = (uint32_t)options.memory_descs.size();
    for (size_t i = 0; i < options.memory_descs.size(); i++) {
        auto &desc = options.memory_descs[i];
        auto &segment = physical_segment::segment(i);
        segment.initialize(desc.physical_address, desc.size_bytes);
    }
}
