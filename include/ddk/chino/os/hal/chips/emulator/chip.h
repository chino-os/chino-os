// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <array>
#include <chino/units.h>
#include <chrono>
#include <cstddef>
#include <cstdint>

namespace chino::os::hal {
class emulator_chip {
  public:
    inline static constexpr std::array<size_t, 1> max_memory_segments_sizes{64 * MiB};
    inline static constexpr size_t cpus_count = 1;

    static void enable_systick(uint64_t ticks) noexcept;
    static void debug_print(const char *str) noexcept;
};

using chip_t = emulator_chip;
} // namespace chino::os::hal
