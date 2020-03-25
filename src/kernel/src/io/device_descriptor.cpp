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
#include <chino/io.h>
#include <libfdt.h>

using namespace chino;
using namespace chino::ob;
using namespace chino::io;

result<device_property, error_code> device_descriptor::property(std::string_view name) const noexcept
{
    int len;
    if (auto prop = fdt_getprop_namelen(fdt_, node_, name.data(), name.length(), &len))
        return ok(device_property(prop, len));
    return err(error_code::not_found);
}

uint32_t device_property::uint32(size_t index) const noexcept
{
    return fdt32_to_cpu(reinterpret_cast<const fdt32_t *>(data_)[index]);
}

std::string_view device_property::string(size_t index) const noexcept
{
    return {};
}
