// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <chino/conf/board_desc.h>
#include <chino/os/kernel/io.h>

namespace chino::os::drivers {
class host_console_device : public kernel::io::device {
  public:
    constexpr host_console_device(hal::meta::board_desc::chip::machine::devices::host_console,
                                  kernel::io::device *bottom_device) noexcept {}
};

class host_console_driver {
  public:
    template <class TDevice, class TBottomDevice> using device_t = host_console_device;
};
} // namespace chino::os::drivers
