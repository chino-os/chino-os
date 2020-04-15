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
#include <chino/ddk/io.h>
#include <chino/io.h>
#include <chino/io/io_manager.h>
#include <cstring>
#include <libfdt.h>

using namespace chino;
using namespace chino::ob;
using namespace chino::io;

result<device_property, error_code> device_descriptor::property(std::string_view name) const noexcept
{
    int len;
    if (auto prop = fdt_getprop_namelen(fdt(), node(), name.data(), name.length(), &len))
        return ok<device_property>(prop, len);
    return err(error_code::not_found);
}

bool device_descriptor::has_compatible() const noexcept
{
    return fdt_get_property(fdt(), node(), "compatible", nullptr);
}

const void *device_descriptor::fdt() const noexcept
{
    return machine_desc_.fdt;
}

uint32_t device_descriptor::address_cells() const noexcept
{
    return fdt_address_cells(fdt(), node());
}

uint32_t device_descriptor::size_cells() const noexcept
{
    return fdt_size_cells(fdt(), node());
}

result<device_descriptor, error_code> device_descriptor::parent() const noexcept
{
    auto pnode = fdt_parent_offset(fdt(), node());
    if (pnode >= 0)
        return ok<device_descriptor>(pnode);
    return err(error_code::not_found);
}

result<device_descriptor, error_code> device_descriptor::first_subnode() const noexcept
{
    auto child = fdt_first_subnode(fdt(), node());
    if (child >= 0)
        return ok<device_descriptor>(child);
    return err(error_code::not_found);
}

result<device_descriptor, error_code> device_descriptor::next_subnode(int prev) const noexcept
{
    auto child = fdt_next_subnode(fdt(), prev);
    if (child >= 0)
        return ok<device_descriptor>(child);
    return err(error_code::not_found);
}

result<std::pair<uint64_t, uint64_t>, error_code> device_descriptor::reg(size_t index) const noexcept
{
    try_var(p, parent());
    auto addr_cells = p.address_cells();
    auto size_cells = p.size_cells();

    try_var(prop, property("reg"));
    auto start = reinterpret_cast<const fdt32_t *>(prop.data()) + (addr_cells + size_cells) * index;

    uint64_t addr = 0, size = 0;
    for (size_t i = 0; i < addr_cells; i++)
        addr = (addr << 32) | fdt32_to_cpu(*start++);
    for (size_t i = 0; i < size_cells; i++)
        size = (size << 32) | fdt32_to_cpu(*start++);
    return ok<std::pair<uint64_t, uint64_t>>(addr, size);
}

uint32_t device_property::uint32(size_t index) const noexcept
{
    return fdt32_to_cpu(reinterpret_cast<const fdt32_t *>(data_)[index]);
}

uint64_t device_property::uint64(size_t index) const noexcept
{
    return fdt64_to_cpu(reinterpret_cast<const fdt64_t *>(data_)[index]);
}

std::string_view device_property::string(size_t index) const noexcept
{
    auto len = len_;
    size_t cnt = 0;
    auto cnt_idx = 0;
    while (cnt < len)
    {
        auto ptr = reinterpret_cast<const char *>(data_) + cnt;
        auto str_len = std::strlen(ptr);
        if (cnt_idx++ == index)
            return { ptr, str_len };
        cnt += str_len + 1;
    }

    return {};
}

const driver_id *driver::check_compatible(std::string_view compatible) const noexcept
{
    for (auto &id : match_table)
    {
        if (id.compatible == compatible)
            return &id;
    }

    return nullptr;
}
