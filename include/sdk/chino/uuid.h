// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "compiler.h"
#include <array>
#include <bit>

namespace chino {
class uuid {
  public:
    constexpr uuid(uint32_t data1, uint16_t data2, uint16_t data3, std::array<uint8_t, 8> data4) noexcept
        : data1_(data1), data2_(data2), data3_(data3), data4_(data4) {}

    constexpr uint32_t data1() const noexcept { return data1_; }
    constexpr uint16_t data2() const noexcept { return data2_; }
    constexpr uint16_t data3() const noexcept { return data3_; }
    constexpr std::array<uint8_t, 8> data4() const noexcept { return data4_; }

    constexpr std::array<uint8_t, 16> as_bytes() const noexcept {
        return std::bit_cast<std::array<uint8_t, 16>>(*this);
    }

  private:
    uint32_t data1_;
    uint16_t data2_;
    uint16_t data3_;
    std::array<uint8_t, 8> data4_;
};
} // namespace chino
