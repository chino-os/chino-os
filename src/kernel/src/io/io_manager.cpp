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
#include <chino/ddk/kernel.h>
#include <chino/io.h>
#include <chino/io/io_manager.h>
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

machine_desc io::machine_desc_;

static void setup_machine_desc(const void *fdt, int node) noexcept
{
    device_descriptor root(node);
    machine_desc_.fdt = fdt;
    machine_desc_.model = root.property("model").unwrap().string();
}

static result<void, error_code> create_device_node(const device_descriptor &node) noexcept
{
    if (node.has_compatible())
    {
    }

    return ok();
}

static uint32_t calc_total_device_nodes(const device_descriptor &root) noexcept
{
    uint32_t count = 0;
    int child;
    fdt_for_each_subnode(child, root.fdt(), root.node())
    {
        if (device_descriptor(child).has_compatible())
            count++;
    }

    return count;
}

machine_desc io::get_machine_desc() noexcept
{
    return machine_desc_;
}

result<void, error_code> kernel::io_manager_init(gsl::span<const uint8_t> fdt)
{
    if (fdt_check_full(fdt.data(), fdt.length_bytes()) != 0)
        return err(error_code::invalid_argument);

    auto root_node = fdt_next_node(fdt.data(), -1, nullptr);
    setup_machine_desc(fdt.data(), root_node);

    auto total_nodes = calc_total_device_nodes(root_node);
    int child;
    fdt_for_each_subnode(child, fdt.data(), root_node)
        try_(create_device_node({ child }));
    return ok();
}
