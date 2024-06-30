// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <array>
#include <chino/units.h>
#include <cstddef>
#include <cstdint>

namespace chino::os::kernel::hal {
class emulator_chip {
  public:
    inline static constexpr std::array<size_t, 1> max_memory_segments_sizes{64 * MiB};
};

using chip_t = emulator_chip;
} // namespace chino::os::kernel::hal
