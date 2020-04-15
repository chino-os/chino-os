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
#include <chino/arch/arm/armv7-m/core_debug.h>

using namespace chino;
using namespace chino::arch;

typedef struct
{
    uint32_t cdbg_en : 1;
    uint32_t halt : 1;
    uint32_t step : 1;
    uint32_t hsi_trim : 5;
} cdbg_dhcsr_t;

typedef struct
{
    uint32_t vc_core_reset : 1;
    uint32_t reserved0 : 3;
    uint32_t vc_mm_err : 1;
    uint32_t vc_no_cperr : 1;
    uint32_t vc_chk_err : 1;
    uint32_t vc_stat_err : 1;
    uint32_t vc_bus_err : 1;
    uint32_t vc_int_err : 1;
    uint32_t vc_hard_err : 1;
    uint32_t reserved1 : 5;
    uint32_t mon_en : 1;
    uint32_t mon_pend : 1;
    uint32_t mon_step : 1;
    uint32_t mon_req : 1;
    uint32_t trace_ena : 1;
} cdbg_demcr_t;

typedef struct
{
    reg_t<cdbg_dhcsr_t> chcsr;
    reg_t<uint32_t> dcrsr;
    reg_t<uint32_t> dcrdr;
    reg_t<cdbg_demcr_t> demcr;
} cdbg_t;

static volatile cdbg_t &cdbg_r() noexcept { return *reinterpret_cast<volatile cdbg_t *>(CoreDebug_BASE); }

bool core_debug::is_enabled() noexcept
{
    return cdbg_r().chcsr.reg_mut().cdbg_en;
}

void core_debug::monitor_enable() noexcept
{
    cdbg_r().demcr.reg_mut().mon_en = 1;
}

void core_debug::monitor_enable_step() noexcept
{
    cdbg_r().demcr.reg_mut().mon_step = 1;
}

void core_debug::trace_enable() noexcept
{
    cdbg_r().demcr.reg_mut().trace_ena = 1;
}
