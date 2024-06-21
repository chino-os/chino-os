// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "memory_manager.h"
#include "free_page_list.h"
#include "zone.h"

using namespace chino::os::kernel;

namespace {
constinit mm::free_page_list free_physical_page_list_;
constinit mm::zone zones_[1];
} // namespace

void mm::initialize_phase0(const boot_options &options) { free_physical_page_list_.initialize(options.memory_descs); }
