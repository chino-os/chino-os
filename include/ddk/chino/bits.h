// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <cstddef>
#include <cstdint>

namespace chino {
template <class TDest, class TMask> void set_bits(TDest &dest, TMask mask, bool enable) noexcept {
    if (enable)
        dest |= mask;
    else
        dest &= ~mask;
}
} // namespace chino
