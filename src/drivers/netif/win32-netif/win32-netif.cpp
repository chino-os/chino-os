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
#include <pcap.h>

using namespace chino;
using namespace chino::io;

namespace
{
class win32_netif_dev : public device_extension
{
public:
    win32_netif_dev(std::string_view root);

    result<file *, error_code> open(std::string_view filename, create_disposition create_disp) noexcept;
    result<size_t, error_code> read(file &file, gsl::span<gsl::byte> buffer) noexcept;
    result<void, error_code> write(file &file, gsl::span<const gsl::byte> buffer) noexcept;

private:
    std::string_view root_;
};

struct win32_netif_file : public file_extension
{
    HANDLE handle;
};

result<void, error_code> netif_add_device(const driver &drv, const device_id &dev_id);
result<file *, error_code> netif_open_device(const driver &drv, device &dev, std::string_view filename, create_disposition create_disp);
result<void, error_code> netif_close_device(const driver &drv, device &dev, file &file);
result<size_t, error_code> netif_read_device(const driver &drv, device &dev, file &file, gsl::span<gsl::byte> buffer);
result<void, error_code> netif_write_device(const driver &drv, device &dev, file &file, gsl::span<const gsl::byte> buffer);

const driver_id match_table[] = {
    { .compatible = "win32,netif" }
};

const driver netif_drv = {
    .name = "win32-netif",
    .ops = { .add_device = netif_add_device, .open_device = netif_open_device, .close_device = netif_close_device, .read_device = netif_read_device, .write_device = netif_write_device },
    .match_table = match_table
};
EXPORT_DRIVER(netif_drv);

result<void, error_code> netif_add_device(const driver &drv, const device_id &dev_id)
{
    pcap_if_t *alldevs;
    char errbuf[PCAP_ERRBUF_SIZE];

    if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, nullptr, &alldevs, errbuf) != 0)
        return err(error_code::unknown);
    //device_descriptor desc(dev_id.node());
    //auto root = desc.property("win32,path").unwrap().string();
    //try_var(con, create_device(dev_id, device_type::fs, sizeof(win32_netif_dev)));
    //new (&con->extension()) win32_netif_dev(root);
    return ok();
}

result<file *, error_code> netif_open_device(const driver &drv, device &dev, std::string_view filename, create_disposition create_disp)
{
    return dev.extension<win32_netif_dev>().open(filename, create_disp);
}

result<void, error_code> netif_close_device(const driver &drv, device &dev, file &file)
{
    if (CloseHandle(file.extension<win32_netif_file>().handle))
        return ok();
    return err(error_code::unknown);
}

result<size_t, error_code> netif_read_device(const driver &drv, device &dev, file &file, gsl::span<gsl::byte> buffer)
{
    return dev.extension<win32_netif_dev>().read(file, buffer);
}

result<void, error_code> netif_write_device(const driver &drv, device &dev, file &file, gsl::span<const gsl::byte> buffer)
{
    return dev.extension<win32_netif_dev>().write(file, buffer);
}
}

win32_netif_dev::win32_netif_dev(std::string_view root)
{
    CreateDirectoryA(root.data(), nullptr);
    auto error = GetLastError();
    if (error != ERROR_SUCCESS && error != ERROR_ALREADY_EXISTS)
        panic("cannot create win32 root directory");

    auto len = GetFullPathNameA(root.data(), 0, nullptr, nullptr);
    auto path = (char *)malloc(len);
    assert(GetFullPathNameA(root.data(), len, path, nullptr) == len - 1);
    root_ = { path, len - 1 };
}

result<file *, error_code> win32_netif_dev::open(std::string_view filename, create_disposition create_disp) noexcept
{
    auto len = root_.length() + 1 + filename.length() + 1;
    auto fullname = (char *)malloc(len);
    snprintf(fullname, len, "%s\\%s", root_.data(), filename.data());
    auto handle = CreateFileA(fullname, GENERIC_READ | GENERIC_WRITE, 0, nullptr, (DWORD)create_disp, 0, nullptr);
    free(fullname);
    if (handle == INVALID_HANDLE_VALUE)
        return err(error_code::not_found);
    try_var(f, io::create_file(dev(), sizeof(win32_netif_file)));
    f->extension<win32_netif_file>().handle = handle;
    return ok(f);
}

result<size_t, error_code> win32_netif_dev::read(file &file, gsl::span<gsl::byte> buffer) noexcept
{
    DWORD read = 0;
    if (ReadFile(file.extension<win32_netif_file>().handle, buffer.data(), buffer.length_bytes(), &read, nullptr))
        return ok<size_t>(read);
    return err(error_code::io_error);
}

result<void, error_code> win32_netif_dev::write(file &file, gsl::span<const gsl::byte> buffer) noexcept
{
    if (WriteFile(file.extension<win32_netif_file>().handle, buffer.data(), buffer.length_bytes(), nullptr, nullptr))
        return ok();
    return err(error_code::io_error);
}
