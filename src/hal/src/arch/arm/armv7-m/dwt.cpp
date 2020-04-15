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
#include <chino/arch/arm/armv7-m/dwt.h>

using namespace chino;
using namespace chino::arch;

typedef struct
{
    uint32_t cdbg_en : 1;
    uint32_t halt : 1;
    uint32_t step : 1;
    uint32_t hsi_trim : 5;
} dwt_ctrl_t;

typedef struct
{
    uint32_t mask : 3;
} dwt_mask_t;

typedef struct
{
    uint32_t function : 4;
    uint32_t reserved0 : 1;
    uint32_t emit_range : 1;
    uint32_t reserved1 : 1;
    uint32_t cyc_match : 1;
    uint32_t data_value_match : 1;
    uint32_t lnk1_en : 1;
    uint32_t data_value_size : 2;
} dwt_function_t;

typedef struct
{
    reg_t<uint32_t> comp;
    reg_t<dwt_mask_t> mask;
    reg_t<dwt_function_t> function;
    uint32_t reserved0;
} dwt_slot_t;

typedef struct
{
    reg_t<uint32_t> ctrl;
    reg_t<uint32_t> cyc_cnt;
    reg_t<uint32_t> cpi_cnt;
    reg_t<uint32_t> exc_cnt;
    reg_t<uint32_t> sleep_cnt;
    reg_t<uint32_t> lsu_cnt;
    reg_t<uint32_t> fold_cnt;
    reg_t<uint32_t> pcsr;
    dwt_slot_t slots[4];
} dwt_t;

static volatile dwt_t &dwt_r() noexcept { return *reinterpret_cast<volatile dwt_t *>(DWT_BASE); }

void dwt::slot_set_address(uint32_t slot, uintptr_t address) noexcept
{
    dwt_r().slots[slot].comp.raw(address);
}

void dwt::slot_set_mask(uint32_t slot, uint32_t mask) noexcept
{
    dwt_r().slots[slot].mask.reg_mut().mask = mask;
}

void dwt::slot_set_function(uint32_t slot, slot_function_t func, data_size_t data_size) noexcept
{
    auto reg = dwt_r().slots[slot].function.reg();
    reg.function = (uint32_t)func;
    reg.data_value_size = (uint32_t)data_size;
    dwt_r().slots[slot].function.reg(reg);
}
