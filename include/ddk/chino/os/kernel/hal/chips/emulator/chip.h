// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <array>
#include <chino/units.h>
#include <chrono>
#include <cstddef>
#include <cstdint>

namespace chino::os::kernel::hal {
class emulator_chip {
  public:
    inline static constexpr std::array<size_t, 1> max_memory_segments_sizes{64 * MiB};
    inline static constexpr size_t cpus_count = 1;

    static constexpr uint64_t duration_to_cpu_ticks(size_t cpu_id, std::chrono::milliseconds duration) noexcept {
        return duration.count() * 10000; // in 100ns
    }
};

using chip_t = emulator_chip;
} // namespace chino::os::kernel::hal
