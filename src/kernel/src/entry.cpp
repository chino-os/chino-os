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
#include <chino/directory.h>
#include <chino/kernel.h>
#include <chino/object.h>
#include <chino/threading/process.h>
#include <chino/threading/scheduler.h>
#ifdef _MSC_VER
#include <intrin.h>
#endif

using namespace chino;
using namespace chino::threading;

void chino::panic(std::string_view message) noexcept
{
#ifdef _MSC_VER
    __debugbreak();
#endif
    while (1)
        ;
}

result<void, error_code> kernel::kernel_main()
{
    try_(kernel_process_init());
    current_sched().start();

    // Should not reach here
    while (1)
        ;
}

uint32_t kernel::kernel_system_thread_main(void *arg)
{
    kernel::io_manager_init(board::board_t::device_tree());
    auto mem_info = get_system_memory_info();
    return 0;
}
