// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "physical_allocator.h"
#include "physical_segment.h"
#include <chino/os/kernel/hal/chip/chip.h>

using namespace chino::os::kernel;
using namespace chino::os::kernel::mm;

namespace {
namespace detail {
template <class TMaxZoneSizes> struct physical_zone_impl;

template <size_t... MaxZoneSizes> struct physical_zones_impl<std::integer_sequence<size_t, MaxZoneSizes...>> {};
} // namespace detail
} // namespace

void physical_allocator::initialize_phase0(const boot_options &options) {}
