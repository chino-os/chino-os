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
class serial_console_dev : public device_extension
{
public:
    serial_console_dev(file *file)
        : file_(file) {}

    result<size_t, error_code> read(gsl::span<gsl::byte> buffer) noexcept;
    result<void, error_code> write(gsl::span<const gsl::byte> buffer) noexcept;

private:
    file *file_;
};

result<void, error_code> con_attach_device(const driver &drv, device &bottom_dev, std::string_view args);
result<file *, error_code> con_open_device(const driver &drv, device &dev);
result<size_t, error_code> con_read_device(const driver &drv, device &dev, file &file, gsl::span<gsl::byte> buffer);
result<void, error_code> con_write_device(const driver &drv, device &dev, file &file, gsl::span<const gsl::byte> buffer);

const driver con_drv = {
    .type = driver_type::console,
    .name = "serial-console",
    .ops = { .attach_device = con_attach_device, .open_device = con_open_device, .read_device = con_read_device, .write_device = con_write_device },
};
EXPORT_DRIVER(con_drv);

result<void, error_code> con_attach_device(const driver &drv, device &bottom_dev, std::string_view args)
{
    try_var(file, io::open_file(bottom_dev, access_mask::generic_all));
    try_var(con, create_device("console", drv, device_type::console, sizeof(serial_console_dev)));
    new (&con->extension()) serial_console_dev(file);
    return ok();
}

result<file *, error_code> con_open_device(const driver &drv, device &dev)
{
    return create_file(dev, 0);
}

result<size_t, error_code> con_read_device(const driver &drv, device &dev, file &file, gsl::span<gsl::byte> buffer)
{
    return dev.extension<serial_console_dev>().read(buffer);
}

result<void, error_code> con_write_device(const driver &drv, device &dev, file &file, gsl::span<const gsl::byte> buffer)
{
    return dev.extension<serial_console_dev>().write(buffer);
}
}

result<size_t, error_code> serial_console_dev::read(gsl::span<gsl::byte> buffer) noexcept
{
    try_var(ret, read_file(*file_, buffer));

    for (size_t i = 0; i < ret; i++)
    {
        if (buffer[i] == gsl::to_byte('\r'))
            buffer[i] = gsl::to_byte('\n');
        else if (buffer[i] == gsl::to_byte(127))
            buffer[i] = gsl::to_byte('\b');
    }
    return ok(ret);
}

result<void, error_code> serial_console_dev::write(gsl::span<const gsl::byte> buffer) noexcept
{
    return write_file(*file_, buffer);
}
