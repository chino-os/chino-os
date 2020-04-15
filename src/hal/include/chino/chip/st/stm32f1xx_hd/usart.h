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
#include <cstdint>
#include <cstddef>

namespace chino::chip::usart
{
enum class parity_t
{
    none = 0,
    even = 1,
    odd = 2
};

enum class data_bits_t
{
    data_8b = 0,
    data_9b = 1
};

enum class stop_bits_t
{
    stop_1b = 0,
    stop_0p5b = 1,
    stop_2b = 2,
    stop_1p5b = 3
};

void enable(uintptr_t base) noexcept;
void set_format(uintptr_t base, data_bits_t data_bits, stop_bits_t stop_bits, parity_t parity) noexcept;
void set_baud_rate(uintptr_t base, uint32_t apb_clk, uint32_t baud) noexcept;

void tx_enable(uintptr_t base) noexcept;
bool tx_is_empty(uintptr_t base) noexcept;
void tx_send(uintptr_t base, uint8_t data) noexcept;

void rx_enable(uintptr_t base) noexcept;
bool rx_is_not_empty(uintptr_t base) noexcept;
uint8_t rx_recv(uintptr_t base) noexcept;
}
