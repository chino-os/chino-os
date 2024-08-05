// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../device.h"
#include <chino/devices/bluetooth/ble.h>

namespace chino::os::kernel::io {
enum class io_frame_ble_kind : uint16_t {
    dummy,
};

#define BLE_ADV_IND 0b0000
#define BLE_ADV_DIRECT_IND 0b0001
#define BLE_ADV_NONCONN_IND 0b0010
#define BLE_SCAN_REQ 0b0011
#define BLE_SCAN_RSP 0b0100
#define BLE_CONNECT_IND 0b0101

struct ble_advertising_packet {};

struct io_frame_ble_params {
    union {
        struct {
        } dummy;
    };

    template <io_frame_ble_kind Minor> auto &by_minor() noexcept {
        if constexpr (Minor == io_frame_ble_kind::dummy) {
            return dummy;
        } else {
            return;
        }
    }
};

template <> struct io_frame_minor_kind_traits<io_frame_major_kind::ble> {
    using minor_type = io_frame_ble_kind;
    using params_type = io_frame_ble_params;
};

class ble_device : public device {
  public:
    CHINO_DEFINE_KERNEL_OBJECT_KIND(device, object_kind_ble_device);

    virtual result<void> start_watch_advertisement(chino::devices::bluetooth::ble_advertisement_callback_t callback,
                                                   void *callback_arg) noexcept = 0;
    virtual void stop_watch_advertisement() noexcept = 0;
};
} // namespace chino::os::kernel::io
