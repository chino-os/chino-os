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
#include <chino/chip/st/stm32f1xx_hd/platform.h>
#include <chino/chip/st/stm32f1xx_hd/rcc.h>

using namespace chino;
using namespace chino::arch;
using namespace chino::chip;

typedef struct
{
    uint32_t hsi_on : 1;
    uint32_t hsi_rdy : 1;
    uint32_t reserved0 : 1;
    uint32_t hsi_trim : 5;
} rcc_cr_t;

typedef struct
{
    uint32_t reserved;
} rcc_cfgr_t;

typedef struct
{
    uint32_t reserved;
} rcc_cir_t;

typedef struct
{
    uint32_t reserved;
} rcc_apb2_rstr_t;

typedef struct
{
    uint32_t reserved;
} rcc_apb1_rstr_t;

typedef struct
{
    uint32_t reserved;
} rcc_ahb_enr_t;

typedef struct
{
    uint32_t afio_en : 1;
    uint32_t reserved0 : 1;
    uint32_t iop_a_en : 1;
    uint32_t iop_b_en : 1;
} rcc_apb2_enr_t;

typedef struct
{
    reg_t<rcc_cr_t> cr;
    reg_t<rcc_cfgr_t> cfgr;
    reg_t<rcc_cir_t> cir;
    reg_t<rcc_apb2_rstr_t> apb2_rstr;
    reg_t<rcc_apb1_rstr_t> apb1_rstr;
    reg_t<rcc_ahb_enr_t> ahb_enr;
    reg_t<rcc_apb2_enr_t> apb2_enr;
} rcc_t;

static volatile rcc_t &rcc_r() noexcept { return *reinterpret_cast<volatile rcc_t *>(RCC_BASE); }

void rcc::clock_enable(apb2_periph periph) noexcept
{
    rcc_r().apb2_enr.set(1 << (uint32_t)periph);
}
