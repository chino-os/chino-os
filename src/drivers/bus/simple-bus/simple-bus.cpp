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

using namespace chino;
using namespace chino::io;

namespace
{
result<void, error_code> sb_add_device(const driver &drv, const device_id &dev_id);

const driver_id match_table[] = {
    { .compatible = "simple-bus" }
};

const driver sb_drv = {
    .name = "simple-bus",
    .ops = { .add_device = sb_add_device },
    .match_table = match_table
};
EXPORT_DRIVER(sb_drv);

result<void, error_code> sb_add_device(const driver &drv, const device_id &dev_id)
{
    try_var(bus, create_device(dev_id, device_type::bus, 0));
    return populate_sub_devices(*bus);
}
}
