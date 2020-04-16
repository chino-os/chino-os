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
#include <chino/chip/wm/w600/uart.h>

using namespace chino;
using namespace chino::chip;

typedef struct
{
    uint32_t uart_line_ctrl;
    uint32_t auto_flow_ctrl;
    uint32_t dma_ctrl;
    uint32_t uart_fifo_ctrl;
    uint32_t baud_rate_ctrl;
    uint32_t int_mask;
    uint32_t int_src;
    uint32_t fifo_status;
    uint32_t tx_data;
    uint32_t reserved0[3];
    uint32_t rx_data;
} uart_t;

static volatile uart_t &uart_r(uintptr_t base) noexcept { return *reinterpret_cast<volatile uart_t *>(base); }

void uart::set_baud_rate(uintptr_t base, uint32_t apb_clk, uint32_t baudrate) noexcept
{
    uart_r(base).uart_line_ctrl = 0xC3;
    uart_r(base).auto_flow_ctrl = 0x14;
    uart_r(base).dma_ctrl = 0x0;
    uart_r(base).baud_rate_ctrl = 0x81;
    //auto ubdiv = apb_clk / (16 * baudrate) - 1;
    //auto ubdiv_frac = (apb_clk % (baudrate * 16)) / baudrate;
    //uart_r(base).baud_rate_ctrl = (ubdiv_frac << 16) | ubdiv;
}

bool uart::is_tx_fifo_full(uintptr_t base) noexcept
{
    return (uart_r(base).fifo_status & 0x3F) == 0x3F;
}

void uart::write(uintptr_t base, uint8_t data) noexcept
{
    uart_r(base).tx_data = data;
}
