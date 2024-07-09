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
#include <chino/arch/reg.h>
#include <chino/chip/st/stm32f1xx_hd/gpio.h>

using namespace chino;
using namespace chino::arch;
using namespace chino::chip;

typedef struct
{
    reg_t<uint32_t> cr_l;
    reg_t<uint32_t> cr_h;
    reg_t<uint32_t> idr;
    reg_t<uint32_t> odr;
    reg_t<uint32_t> bsrr;
    reg_t<uint32_t> brr;
} gpio_t;

static volatile gpio_t &gpio_r(uintptr_t base) noexcept { return *reinterpret_cast<volatile gpio_t *>(base); }
static bits_range cr_range(uint32_t pin) noexcept { return { .start = pin * 4, .end = (pin + 1) * 4 }; }
static constexpr uint32_t pin_idx(uint32_t pin) noexcept { return pin > 7 ? pin - 8 : pin; }

void gpio::pin_set_mode(uintptr_t base, uint32_t pin, input_mode_t mode) noexcept
{
    auto cr_val = (mode == input_mode_t::in_pull_down ? 0b10 : (uint32_t)mode) << 2;
    auto &cr = pin > 7 ? gpio_r(base).cr_h : gpio_r(base).cr_l;

    cr.bits(cr_range(pin_idx(pin)), cr_val);
    if (mode == input_mode_t::in_pull_up)
        pins_set(base, 1 << pin);
    else if (mode == input_mode_t::in_pull_down)
        pins_clear(base, 1 << pin);
}

void gpio::pin_set_mode(uintptr_t base, uint32_t pin, output_mode_t mode, speed_t speed) noexcept
{
    auto cr_val = (uint32_t)mode;
    cr_val = (cr_val << 2) | (uint32_t)speed;
    auto &cr = pin > 7 ? gpio_r(base).cr_h : gpio_r(base).cr_l;

    cr.bits(cr_range(pin_idx(pin)), cr_val);
}

void gpio::pins_set(uintptr_t base, uint32_t pin_mask) noexcept
{
    gpio_r(base).bsrr.raw(pin_mask);
}

void gpio::pins_clear(uintptr_t base, uint32_t pin_mask) noexcept
{
    gpio_r(base).brr.raw(pin_mask);
}
