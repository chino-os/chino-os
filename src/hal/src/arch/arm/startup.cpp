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
#include <chino_config.h>
#include <chino/ddk/kernel.h>
#include <chino/ddk/utility.h>

using namespace chino;
using namespace chino::arch;
using namespace chino::kernel;

extern "C"
{
    extern uint8_t _heap_start[];
    extern uint8_t _heap_end[];
	extern void __libc_init_array(void);
	extern void __libc_fini_array(void);
}

extern "C" void chinoStartup()
{
	//__libc_init_array();
    board::board_t::boot_print_init();
    while (1)
        board::board_t::boot_print("Hello\n");

    auto mem_beg = (uint8_t *)align(uintptr_t(_heap_start), PAGE_SIZE);
    auto mem_end = (uint8_t *)align_down(uintptr_t(_heap_end), PAGE_SIZE);
    
    //physical_memory_desc mem_desc = {
    //    .runs_count = 1,
    //    .pages_count = (mem_end - mem_beg) / PAGE_SIZE,
    //    .runs = {
    //        { mem_beg, (mem_end - mem_beg) / PAGE_SIZE } }
    //};
//
    //memory_manager_init(mem_desc)
    //    .expect("Cannot init memory manager");
    while (1);
}
