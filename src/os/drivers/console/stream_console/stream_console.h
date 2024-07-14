// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/conf/board_desc.h>
#include <chino/os/kernel/io.h>

namespace chino::os::drivers {
class stream_console_device : public device {
  public:
    constexpr stream_console_device(hal::meta::board_desc::chip::machine::devices::stream_console) noexcept {}

    std::string_view name() const noexcept override {
        using namespace std::string_view_literals;
        return "console"sv;
    }

    result<void> install(os::device &stream_device) noexcept;

    result<object_ptr<file>> open(std::string_view path, create_disposition create_disposition) noexcept override;
    result<void> close(file &file) noexcept override;

    result<size_t> read(file &file, std::span<const iovec> iovs, size_t offset) noexcept override;
    result<size_t> write(file &file, std::span<const iovec> iovs, size_t offset) noexcept override;
    result<size_t> control(file &file, control_code_t code, std::span<const std::byte> in_buffer,
                           std::span<std::byte> out_buffer) noexcept override;

  private:
    object_ptr<file> stream_file_;
};

class stream_console_driver {
  public:
    template <class TDevice, class TBottomDevice> using device_t = stream_console_device;

    static result<void> install_device(stream_console_device &device, os::device &stream_device) noexcept;
};
} // namespace chino::os::drivers
