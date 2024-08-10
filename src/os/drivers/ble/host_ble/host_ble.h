// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../../../hal/archs/emulator/emulator.h"
#include <chino/conf/board_desc.h>
#include <chino/os/kernel/io.h>
#include <chino/os/kernel/io/devices/ble_device.h>
#include <chino/os/kernel/ps.h>

namespace chino::os::drivers {
class host_ble_device : public kernel::io::ble_device {
  public:
    constexpr host_ble_device(hal::meta::board_desc::chip::machine::devices::host_ble) noexcept {}

    std::string_view name() const noexcept override {
        using namespace std::string_view_literals;
        return "ble"sv;
    }

    result<void> install() noexcept;

    result<void> close(kernel::io::file &file) noexcept override;

    result<void> start_watch_advertisement(chino::devices::bluetooth::ble_advertisement_callback_t callback,
                                           void *callback_arg) noexcept override;
    void stop_watch_advertisement() noexcept override;
};

class host_ble_driver {
  public:
    template <class TDevice, class TBottomDevice> using device_t = host_ble_device;

    static result<void> install_device(host_ble_device &device) noexcept;
};
} // namespace chino::os::drivers
