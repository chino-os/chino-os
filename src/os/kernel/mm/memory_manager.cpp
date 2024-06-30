// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "memory_manager.h"
#include "physical/physical_allocator.h"

using namespace chino::os::kernel;

namespace {} // namespace

void mm::initialize_phase0(const boot_options &options) noexcept { physical_allocator::initialize_phase0(options); }
