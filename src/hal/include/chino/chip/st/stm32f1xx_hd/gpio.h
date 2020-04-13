// MIT License
//
// Copyright (c) 2020 SunnyCase
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#pragma once
#include <cstddef>
#include <cstdint>

namespace chino::chip::gpio
{
enum class input_mode_t : uint32_t
{
    analog = 0,
    in_floating = 1,
    in_pull_up = 2,
    in_pull_down = 3
};

enum class output_mode_t : uint32_t
{
    gp_out_push_pull = 0,
    gp_out_open_drain = 1,
    af_out_push_pull = 2,
    af_out_open_drain = 3
};

enum class speed_t : uint32_t
{
    out_10MHz = 0b01,
    out_2MHz = 0b10,
    out_50MHz = 0b11
};

void pin_set_mode(uintptr_t base, uint32_t pin, input_mode_t mode) noexcept;
void pin_set_mode(uintptr_t base, uint32_t pin, output_mode_t mode, speed_t speed) noexcept;
void pins_set(uintptr_t base, uint32_t pin_mask) noexcept;
void pins_clear(uintptr_t base, uint32_t pin_mask) noexcept;
}
