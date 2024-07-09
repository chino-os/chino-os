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
#include <chino/chip/st/stm32f1xx_hd/gpio.h>
#include <chino/chip/st/stm32f1xx_hd/rcc.h>
#include <chino/chip/st/stm32f1xx_hd/usart.h>
#include <chino_config.h>
#include <chipdef.h>

using namespace chino;
using namespace chino::chip;
using namespace chino::board;

void board::board_t::boot_print_init() noexcept
{
    using tx_t = config::usart1::pins::tx;
    using rx_t = config::usart1::pins::rx;
    constexpr auto tx_base = tx_t::bank_t::dev_t::base;
    constexpr auto rx_base = rx_t::bank_t::dev_t::base;

    rcc::clock_enable(rcc::periph_from_base_v<tx_base>);
    rcc::clock_enable(rcc::periph_from_base_v<rx_base>);
    rcc::clock_enable(rcc::apb2_periph::usart1);

    gpio::pin_set_mode(tx_base, tx_t::index, gpio::output_mode_t::af_out_push_pull, gpio::speed_t::out_50MHz);
    gpio::pin_set_mode(rx_base, rx_t::index, gpio::input_mode_t::in_floating);

    usart::set_baud_rate(config::usart1::base, 8000000, BOOT_DEBUG_SERIAL_BAUDRATE);
    usart::set_format(config::usart1::base, usart::data_bits_t::data_8b, usart::stop_bits_t::stop_1b, usart::parity_t::none);
    usart::tx_enable(config::usart1::base);
    usart::enable(config::usart1::base);
}

void board::board_t::boot_print(const char *message) noexcept
{
    auto p = message;
    while (*p)
    {
        while (!usart::tx_is_empty(config::usart1::base))
            ;
        usart::tx_send(config::usart1::base, *p++);
    }
}
