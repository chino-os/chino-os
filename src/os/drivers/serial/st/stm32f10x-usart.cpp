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
#include <chino/chip/st/stm32f1xx_hd/usart.h>

using namespace chino;
using namespace chino::chip;
using namespace chino::io;

namespace
{
class stm32f10x_usart_dev : public device_extension
{
public:
    stm32f10x_usart_dev(uintptr_t base)
        : base_(base) {}

    void enable() noexcept;
    result<size_t, error_code> read(gsl::span<gsl::byte> buffer) noexcept;
    result<void, error_code> write(gsl::span<const gsl::byte> buffer) noexcept;

private:
    uintptr_t base_;
};

result<void, error_code> con_add_device(const driver &drv, const device_id &dev_id);
result<file *, error_code> con_open_device(const driver &drv, device &dev, std::string_view filename, create_disposition create_disp);
result<size_t, error_code> con_read_device(const driver &drv, device &dev, file &file, gsl::span<gsl::byte> buffer);
result<void, error_code> con_write_device(const driver &drv, device &dev, file &file, gsl::span<const gsl::byte> buffer);

const driver_id match_table[] = {
    { .compatible = "st,stm32f10x-usart" }
};

const driver con_drv = {
    .name = "stm32f10x-usart",
    .ops = { .add_device = con_add_device, .open_device = con_open_device, .read_device = con_read_device, .write_device = con_write_device },
    .match_table = match_table
};
EXPORT_DRIVER(con_drv);

result<void, error_code> con_add_device(const driver &drv, const device_id &dev_id)
{
    device_descriptor desc(dev_id.node());
    try_var(reg, desc.reg());
    try_var(con, create_device(dev_id, device_type::serial, sizeof(stm32f10x_usart_dev)));
    new (&con->extension()) stm32f10x_usart_dev(uintptr_t(reg.first));
    return ok();
}

result<file *, error_code> con_open_device(const driver &drv, device &dev, std::string_view filename, create_disposition create_disp)
{
    if (filename.empty())
    {
        try_var(file, create_file(dev, 0));
        dev.extension<stm32f10x_usart_dev>().enable();
        return ok(file);
    }

    return err(error_code::invalid_path);
}

result<size_t, error_code> con_read_device(const driver &drv, device &dev, file &file, gsl::span<gsl::byte> buffer)
{
    return dev.extension<stm32f10x_usart_dev>().read(buffer);
}

result<void, error_code> con_write_device(const driver &drv, device &dev, file &file, gsl::span<const gsl::byte> buffer)
{
    return dev.extension<stm32f10x_usart_dev>().write(buffer);
}
}

void stm32f10x_usart_dev::enable() noexcept
{
    usart::rx_enable(base_);
    usart::tx_enable(base_);
    usart::enable(base_);
}

result<size_t, error_code> stm32f10x_usart_dev::read(gsl::span<gsl::byte> buffer) noexcept
{
    size_t read = 0;
    if (buffer.length())
    {
        while (!usart::rx_is_not_empty(base_));
        while (read < buffer.length() && usart::rx_is_not_empty(base_))
            buffer[read++] = (gsl::byte)usart::rx_recv(base_);
    }

    return ok(read);
}

result<void, error_code> stm32f10x_usart_dev::write(gsl::span<const gsl::byte> buffer) noexcept
{
    size_t written = 0;
    while (written < buffer.length())
    {
        while (!usart::tx_is_empty(base_));
        usart::tx_send(base_, (uint8_t)buffer[written++]);
    }

    return ok();
}
