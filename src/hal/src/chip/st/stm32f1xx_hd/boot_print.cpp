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
#include <board.h>
#include <chino/chip/st/stm32f1xx_hd/platform.h>
#include <chino/chip/st/stm32f1xx_hd/usart.h>
#include <chino/chip/st/stm32f1xx_hd/rcc.h>
#include <chino/chip/st/stm32f1xx_hd/gpio.h>
#include <chino_config.h>

using namespace chino;
using namespace chino::chip;
using namespace chino::board;

void board::board_t::boot_print_init() noexcept
{
    rcc::clock_enable(rcc::apb2_periph::iop_a);
    rcc::clock_enable(rcc::apb2_periph::usart_1);

    // tx
    gpio::pin_set_mode(GPIOA_BASE, 9, gpio::output_mode_t::af_out_push_pull, gpio::speed_t::out_50MHz);
    // rx
    gpio::pin_set_mode(GPIOA_BASE, 10, gpio::input_mode_t::in_floating);

    usart::set_baud_rate(USART1_BASE, 8000000, BOOT_DEBUG_SERIAL_BAUDRATE);
    usart::set_format(USART1_BASE, usart::data_bits_t::data_8b, usart::stop_bits_t::stop_1b, usart::parity_t::none);
    usart::tx_enable(USART1_BASE);
    usart::enable(USART1_BASE);
}

void board::board_t::boot_print(const char *message) noexcept
{
    auto p = message;
    while (*p)
    {
        while (!usart::tx_is_empty(USART1_BASE));
        usart::tx_send(USART1_BASE, *p++);
    }
}
