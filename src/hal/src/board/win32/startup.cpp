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
#include "../../arch/win32/target.h"
#include "crtinit.inl"
#include <Windows.h>
#include <board.h>
#include <chino/ddk/kernel.h>
#include <chino_config.h>

using namespace chino;
using namespace chino::arch;
using namespace chino::kernel;

extern "C" _CRTALLOC(".CRT$XIA") _PIFV __xi_a[] = { nullptr }; // C initializers (first)
extern "C" _CRTALLOC(".CRT$XIZ") _PIFV __xi_z[] = { nullptr }; // C initializers (last)
extern "C" _CRTALLOC(".CRT$XCA") _PVFV __xc_a[] = { nullptr }; // C++ initializers (first)
extern "C" _CRTALLOC(".CRT$XCZ") _PVFV __xc_z[] = { nullptr }; // C++ initializers (last)
extern "C" _CRTALLOC(".CRT$XPA") _PVFV __xp_a[] = { nullptr }; // C pre-terminators (first)
extern "C" _CRTALLOC(".CRT$XPZ") _PVFV __xp_z[] = { nullptr }; // C pre-terminators (last)
extern "C" _CRTALLOC(".CRT$XTA") _PVFV __xt_a[] = { nullptr }; // C terminators (first)
extern "C" _CRTALLOC(".CRT$XTZ") _PVFV __xt_z[] = { nullptr }; // C terminators (last)

#pragma comment(linker, "/merge:.CRT=.rdata")

alignas(PAGE_SIZE) static uint8_t memory[1024 * 1024 * 4];

extern "C" void chinoStartup()
{
    // Do C initialization
    int initret = _initterm_e(__xi_a, __xi_z);
    if (initret != 0)
        ExitProcess(255);

    // Do C++ initialization
    _initterm(__xc_a, __xc_z);

    arch_t::init_stack_check();
    physical_memory_desc mem_desc = {
        .runs_count = 1,
        .pages_count = std::size(memory) / PAGE_SIZE,
        .runs = {
            { memory, std::size(memory) / PAGE_SIZE } }
    };

    memory_manager_init(mem_desc)
        .expect("Cannot init memory manager");

    auto ret = kernel_main();
    ExitProcess(ret.is_ok() ? 0 : -1);
}

void board::win32_board::boot_print(const char *message) noexcept
{
    OutputDebugStringA(message);
}
