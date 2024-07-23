// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#define NTDDI_VERSION NTDDI_WIN10_RS5
#include <Windows.h>
#include <chino/conf/board_desc.h>
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/ps.h>

namespace chino::os::drivers {
class host_serial_device : public device {
  public:
    constexpr host_serial_device(hal::meta::board_desc::chip::machine::devices::host_serial) noexcept {}

    std::string_view name() const noexcept override {
        using namespace std::string_view_literals;
        return "host_serial"sv;
    }

    result<void> install(std::string_view port_name) noexcept;

    result<file> open(std::string_view path, create_disposition create_disposition) noexcept override;
    result<void> close(file &file) noexcept override;

    result<size_t> read(file &file, std::span<const iovec> iovs, std::optional<size_t> offset) noexcept override;
    result<size_t> write(file &file, std::span<const iovec> iovs, std::optional<size_t> offset) noexcept override;
    result<size_t> control(file &file, control_code_t code, std::span<const std::byte> in_buffer,
                           std::span<std::byte> out_buffer) noexcept override;

  private:
    static result<void> stdin_irq_handler(hal::arch_irq_number_t, void *context) noexcept;

  private:
    HANDLE port_ = nullptr;
    HANDLE stdout_ = nullptr;
    HANDLE stdin_wait_handle_ = nullptr;
    kernel::ps::event stdin_avail_event_;
};

class host_serial_driver {
  public:
    template <class TDevice, class TBottomDevice> using device_t = host_serial_device;

    static result<void> install_device(host_serial_device &device) noexcept;
};
} // namespace chino::os::drivers
