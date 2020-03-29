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
#pragma comment(linker, "/merge:.CHDRV=.rdata")

__declspec(allocate(".CHDRV$A")) const ::chino::io::driver *drivers_begin_[] = { nullptr };
__declspec(allocate(".CHDRV$Z")) const ::chino::io::driver *drivers_end_[] = { nullptr };
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

static result<void, error_code> populate_devices(int node) noexcept
{
    int child;
    fdt_for_each_subnode(child, machine_desc_.fdt, node)
        try_(probe_device({ child }, nullptr));
    return ok();
}

static result<device_id, error_code> create_device_id(const device_descriptor &node) noexcept
{
    auto compats = node.property("compatible").unwrap();
    size_t compat_id = 0;
    const device_id *dev_id = nullptr;

    while (true)
    {
        auto compat = compats.string(compat_id++);
        if (compat.empty())
            break;
        auto drv = drivers_begin_;
        while (++drv < drivers_end_)
        {
            if (*drv)
            {
                auto id = (*drv)->check_compatible(compat);
                if (id)
                    return ok<device_id>(node.node(), **drv, *id);
            }
        }
    }

    return err(error_code::not_found);
}

machine_desc io::get_machine_desc() noexcept
{
    return machine_desc_;
}

result<void, error_code> io::probe_device(const device_descriptor &node, device *parent) noexcept
{
    if (node.has_compatible())
    {
        auto dev_id_r = create_device_id(node);
        if (dev_id_r.is_err())
            return dev_id_r.unwrap_err();
        auto dev_id = dev_id_r.unwrap();
        try_(dev_id.drv().ops.add_device(dev_id.drv(), dev_id));
    }

    return ok();
}

result<void, error_code> io::populate_sub_devices(device &parent) noexcept
{
    auto parent_id = parent.id.node();

    int child;
    fdt_for_each_subnode(child, machine_desc_.fdt, parent_id)
        try_(probe_device({ child }, &parent));
    return ok();
}

result<void, error_code> kernel::io_manager_init(gsl::span<const uint8_t> fdt)
{
    if (fdt_check_full(fdt.data(), fdt.length_bytes()) != 0)
        return err(error_code::invalid_argument);

    auto root_node = fdt_next_node(fdt.data(), -1, nullptr);
    setup_machine_desc(fdt.data(), root_node);
    return populate_devices(root_node);
}
