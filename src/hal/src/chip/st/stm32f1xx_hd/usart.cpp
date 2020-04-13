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
#include <chino/chip/st/stm32f1xx_hd/usart.h>

using namespace chino;
using namespace chino::arch;
using namespace chino::chip;

typedef struct
{
    uint32_t pe : 1;
    uint32_t fe : 1;
    uint32_t ne : 1;
    uint32_t ore : 1;
    uint32_t idle : 1;
    uint32_t rxne : 1;
    uint32_t tc : 1;
    uint32_t txe : 1;
    uint32_t lbd : 1;
    uint32_t cts : 1;
} usart_sr_t;

typedef struct
{
    uint32_t data : 9;
    uint32_t reserved : 23;
} usart_dr_t;

typedef struct
{
    uint32_t div_frac : 4;
    uint32_t div_mantissa : 12;
    uint32_t reserved : 16;
} usart_brr_t;

typedef struct
{
    uint32_t sbk : 1;
    uint32_t rwu : 1;
    uint32_t re : 1;
    uint32_t te : 1;
    uint32_t idle_ie : 1;
    uint32_t rxne_ie : 1;
    uint32_t tc_ie : 1;
    uint32_t txe_ie : 1;
    uint32_t pe_ie : 1;
    uint32_t ps : 1;
    uint32_t pce : 1;
    uint32_t wake : 1;
    uint32_t m : 1;
    uint32_t ue : 1;
    uint32_t reserved : 18;
} usart_cr1_t;

typedef struct
{
    uint32_t add : 4;
    uint32_t reserved0 : 1;
    uint32_t lbd_len : 1;
    uint32_t lbd_ie : 1;
    uint32_t reserved1 : 1;
    uint32_t lbcl : 1;
    uint32_t cpha : 1;
    uint32_t cpol : 1;
    uint32_t clk_en : 1;
    uint32_t stop : 2;
    uint32_t lin_en : 1;
    uint32_t reserved : 17;
} usart_cr2_t;

typedef struct
{
    reg_t<usart_sr_t> sr;
    reg_t<usart_dr_t> dr;
    reg_t<usart_brr_t> brr;
    reg_t<usart_cr1_t> cr1;
    reg_t<usart_cr2_t> cr2;
    reg_t<usart_brr_t> cr3;
    reg_t<usart_brr_t> gtpr;
} usart_t;

static volatile usart_t &usart_r(uintptr_t base) noexcept { return *reinterpret_cast<volatile usart_t *>(base); }

void usart::set_baud_rate(uintptr_t base, uint32_t apb_clk, uint32_t baudrate) noexcept
{
    auto div = (25 * apb_clk) / (4 * baudrate);
    auto div_frac = div - (div / 100 * 100);

    auto brr = usart_r(base).brr.reg();
    brr.div_mantissa = div / 100;
    brr.div_frac = (div_frac * 16 + 50) / 100;
    usart_r(base).brr.reg(brr);
}

void usart::enable(uintptr_t base) noexcept
{
    usart_r(base).cr1.reg_mut().ue = 1;
}

void usart::set_format(uintptr_t base, data_bits_t data_bits, stop_bits_t stop_bits, parity_t parity) noexcept
{
    auto cr1 = usart_r(base).cr1.reg();
    cr1.m = (uint32_t)data_bits;
    if (parity == parity_t::none)
        cr1.pce = 0;
    else
    {
        cr1.pce = 1;
        cr1.ps = (uint32_t)parity - 1;
    }

    usart_r(base).cr1.reg(cr1);
    usart_r(base).cr2.reg_mut().stop = (uint32_t)stop_bits;
}

void usart::tx_enable(uintptr_t base) noexcept
{
    usart_r(base).cr1.reg_mut().te = 1;
}

bool usart::tx_is_empty(uintptr_t base) noexcept
{
    return usart_r(base).sr.reg().txe;
}

void usart::tx_send(uintptr_t base, uint8_t data) noexcept
{
    usart_r(base).dr.reg({ data });
}
