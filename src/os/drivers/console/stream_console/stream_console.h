// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/conf/board_desc.h>
#include <chino/os/kernel/io.h>

namespace chino::os::drivers {
class stream_console_device : public kernel::io::device {
  public:
    constexpr stream_console_device(hal::meta::board_desc::chip::machine::devices::stream_console) noexcept {}

    std::string_view name() const noexcept override {
        using namespace std::string_view_literals;
        return "console"sv;
    }

    result<void> install(kernel::io::device &stream_device) noexcept;

    result<size_t> fast_read(file &file, std::span<std::byte> buffer, std::optional<size_t> offset) noexcept override;
    result<size_t> fast_write(file &file, std::span<const std::byte> buffer,
                              std::optional<size_t> offset) noexcept override;

  private:
    file *stream_file_;
};

class stream_console_driver {
  public:
    template <class TDevice, class TBottomDevice> using device_t = stream_console_device;

    static result<void> install_device(stream_console_device &device, kernel::io::device &stream_device) noexcept;
};
} // namespace chino::os::drivers
