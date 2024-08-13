// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "../io_manager.h"
#include <chino/os/kernel/io/devices/ble_device.h>
#include <chino/os/kernel/kd.h>
#include <chino/os/kernel/ob.h>

using namespace chino;
using namespace chino::devices::bluetooth;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;
using namespace std::string_view_literals;

constinit static object_ptr<ble_device> default_ble_device_;

result<void> io::initialize_ble_manager(device &ble_device) noexcept {
    kassert(default_ble_device_.empty());
    default_ble_device_ = static_cast<io::ble_device *>(&ble_device);
    try_(default_ble_device_->start_watch_advertisement([](ble_advertisement_received *, void *) { return ok(true); },
                                                        nullptr));
    return ok();
}
