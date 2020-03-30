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
#include <chino/arch/win32/target.h>
#include <chino/ddk/io.h>

using namespace chino;
using namespace chino::io;

namespace
{
class win32_console_dev : public device_extension
{
public:
    win32_console_dev();

private:
    HANDLE stdin_, stdout_;
};

result<void, error_code> con_add_device(const driver &drv, const device_id &dev_id);

const driver_id match_table[] = {
    { .compatible = "win32,console" }
};

const driver con_drv = {
    .name = "win32-console",
    .ops = { .add_device = con_add_device },
    .match_table = match_table
};
EXPORT_DRIVER(con_drv);

result<void, error_code> con_add_device(const driver &drv, const device_id &dev_id)
{
    try_var(con, create_device(dev_id, device_type::serial, sizeof(win32_console_dev)));
    new (&con->extension()) win32_console_dev();
    return ok();
}
}

win32_console_dev::win32_console_dev()
    : stdin_(GetStdHandle(STD_INPUT_HANDLE)), stdout_(GetStdHandle(STD_OUTPUT_HANDLE))
{
}
