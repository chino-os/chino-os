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
//#include <board.h>
#include <cassert>
#include <chino/arch/arm/armv7-m/itm.h>

using namespace chino;
using namespace chino::arch;

typedef union
{
    uint8_t u8;
    uint32_t u32;
} itm_port_t;

typedef struct
{
    itm_port_t port[32];
    uint32_t _reserved0[864];
    uint32_t trace_enable;
    uint32_t _reserved1[15];
    uint32_t trace_priviledge;
    uint32_t _reserved2[15];
    uint32_t trace_ctrl;
} itm_t;

#define itm reinterpret_cast<volatile itm_t *>(ITM_BASE)

bool arch::itm_is_enabled() noexcept
{
    return itm->trace_ctrl & 1;
}

bool arch::itm_port_is_enabled(uint32_t port) noexcept
{
    return itm->trace_enable & (1 << port);
}

void arch::itm_port_write(uint32_t port, uint32_t data) noexcept
{
    //if (itm_is_enabled() && itm_port_is_enabled(port))
    {
        while (itm->port[port].u32 == 0)
            ;
        itm->port[port].u32 = data;
    }
}

void arch::itm_port_write_string(uint32_t port, const char *str) noexcept
{
    while (*str)
        itm_port_write(port, *str++);
}
