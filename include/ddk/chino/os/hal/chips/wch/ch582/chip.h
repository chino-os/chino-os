// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <array>
#include <chino/units.h>
#include <cstddef>
#include <cstdint>

namespace chino::os::hal {
class ch582_chip {
  public:
    inline static constexpr std::array<size_t, 1> max_memory_segments_sizes{32 * KiB};
    inline static constexpr size_t cpus_count = 1;
    inline static constexpr uintptr_t ke_services_address = 0x20000000;

    static void enable_systick(uint64_t ticks) noexcept;
    static void debug_print(const char *str) noexcept;
};

using chip_t = ch582_chip;
} // namespace chino::os::hal
