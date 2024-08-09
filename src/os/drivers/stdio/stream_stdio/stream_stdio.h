// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/conf/board_desc.h>
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/io/devices/stdio_device.h>

namespace chino::os::drivers {
class stream_stdio_device : public kernel::io::stdio_device {
  public:
    constexpr stream_stdio_device(hal::meta::board_desc::chip::machine::devices::stream_console) noexcept {}

    std::string_view name() const noexcept override {
        using namespace std::string_view_literals;
        return "stdio"sv;
    }

    result<void> install(kernel::io::device &stream_device) noexcept;

    result<size_t> fast_read(kernel::io::file &file, std::span<std::byte> buffer,
                             std::optional<size_t> offset) noexcept override;
    result<size_t> fast_write(kernel::io::file &file, std::span<const std::byte> buffer,
                              std::optional<size_t> offset) noexcept override;

    result<void> process_io(kernel::io::io_request &irp) noexcept override;

  private:
    kernel::io::file stream_file_;
};

class stream_stdio_driver {
  public:
    template <class TDevice, class TBottomDevice> using device_t = stream_stdio_device;

    static result<void> install_device(stream_stdio_device &device, kernel::io::device &stream_device) noexcept;
};
} // namespace chino::os::drivers
