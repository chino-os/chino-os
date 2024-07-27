// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../../../hal/archs/emulator/emulator.h"
#include <chino/conf/board_desc.h>
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/ps.h>

namespace chino::os::drivers {
class host_serial_device : public kernel::io::device {
  public:
    constexpr host_serial_device(hal::meta::board_desc::chip::machine::devices::host_serial) noexcept {}

    std::string_view name() const noexcept override {
        using namespace std::string_view_literals;
        return "host_serial"sv;
    }

    result<void> install(std::string_view port_name) noexcept;

    result<size_t> fast_control(file &file, control_code_t code, void *arg) noexcept override;

    result<void> process_io(kernel::io::io_request &irp) noexcept override;

  private:
    static result<void> rx_irq_handler(hal::arch_irq_number_t, void *context) noexcept;
    static result<void> tx_irq_handler(hal::arch_irq_number_t, void *context) noexcept;

  private:
    HANDLE port_ = nullptr;
    hal::irq_overlapped rx_overlapped_ = {};
    hal::irq_overlapped tx_overlapped_ = {};
};

class host_serial_driver {
  public:
    template <class TDevice, class TBottomDevice> using device_t = host_serial_device;

    static result<void> install_device(host_serial_device &device) noexcept;
};
} // namespace chino::os::drivers
