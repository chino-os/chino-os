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
#include <ulog.h>

using namespace chino;
using namespace chino::io;

namespace
{
class win32_netif_dev : public device_extension
{
public:
    win32_netif_dev(pcap_if_t *pcap_if)
        : pcap_if_(pcap_if) {}

    result<file *, error_code> open(std::string_view filename, create_disposition create_disp) noexcept;
    result<size_t, error_code> read(file &file, gsl::span<gsl::byte> buffer) noexcept;
    result<void, error_code> write(file &file, gsl::span<const gsl::byte> buffer) noexcept;

private:
    pcap_if_t *pcap_if_;
};

struct win32_netif_file : public file_extension
{
    pcap_t *pcap;
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

    auto pcap_dev = alldevs;
    while (pcap_dev)
    {
        try_var(netif, create_device(dev_id, device_type::netif, sizeof(win32_netif_dev)));
        new (&netif->extension()) win32_netif_dev(pcap_dev);

        pcap_dev = pcap_dev->next;
    }

    return ok();
}

result<file *, error_code> netif_open_device(const driver &drv, device &dev, std::string_view filename, create_disposition create_disp)
{
    return dev.extension<win32_netif_dev>().open(filename, create_disp);
}

result<void, error_code> netif_close_device(const driver &drv, device &dev, file &file)
{
    pcap_close(file.extension<win32_netif_file>().pcap);
    return ok();
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

result<file *, error_code> win32_netif_dev::open(std::string_view filename, create_disposition create_disp) noexcept
{
    char errbuf[PCAP_ERRBUF_SIZE];
    auto pcap = pcap_open(pcap_if_->name, 65535, PCAP_OPENFLAG_PROMISCUOUS, -1, nullptr, errbuf);
    if (!pcap)
    {
        ULOG_WARNING("Cannot open netif %s, reason: %s\n", pcap_if_->name, errbuf);
        return err(error_code::io_error);
    }

    try_var(f, io::create_file(dev(), sizeof(win32_netif_file)));
    f->extension<win32_netif_file>().pcap = pcap;
    return ok(f);
}

result<size_t, error_code> win32_netif_dev::read(file &file, gsl::span<gsl::byte> buffer) noexcept
{
    pcap_pkthdr *hdr;
    pcap_next_ex(file.extension<win32_netif_file>().pcap, &hdr, nullptr);
    return err(error_code::io_error);
}

result<void, error_code> win32_netif_dev::write(file &file, gsl::span<const gsl::byte> buffer) noexcept
{
    return err(error_code::io_error);
}
