// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/units.h>
#include <cstddef>
#include <cstdint>

namespace chino::os::kernel::hal {
class emulator_cpu {
  public:
    inline static constexpr size_t min_page_size = 4 * KiB;
};

using cpu_t = emulator_cpu;
} // namespace chino::hal
