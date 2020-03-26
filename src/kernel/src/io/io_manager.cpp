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
#include <chino/io/io_manager.h>
#include <chino/kernel.h>
#include <libfdt.h>

using namespace chino;
using namespace chino::ob;
using namespace chino::io;

#ifdef _MSC_VER
#pragma section(".CHINO_DRV$A", read) // Begin drivers
#pragma section(".CHINO_DRV$C", read) // Drivers
#pragma section(".CHINO_DRV$Z", read) // End drivers
#pragma comment(linker, "/merge:.CHINO_DRV=.rdata")

__declspec(allocate(".CHINO_DRV$A")) static const ::chino::io::driver drivers_begin_[1];
__declspec(allocate(".CHINO_DRV$Z")) static const ::chino::io::driver drivers_end_[1];
#else
#error "Unsupported compiler"
#endif

result<void, error_code> kernel::io_manager_init(const void *fdt)
{
    auto first_node = fdt_next_node(fdt, -1, nullptr);
    return ok();
}
