// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../device.h"

namespace chino::os::kernel::io {

enum class io_frame_ble_kind : uint16_t {
    scan,
};

typedef bool (*ble_scan_callback_t)(void *);

struct io_frame_ble_params {
    union {
        struct {
            ble_scan_callback_t callback;
            void *callback_arg;
            uint32_t scan_ms;
        } scan;
    };

    template <io_frame_ble_kind Minor> auto &by_minor() noexcept {
        if constexpr (Minor == io_frame_ble_kind::scan) {
            return scan;
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

    virtual result<void> scan(ble_scan_callback_t callback, void *callback_arg, uint32_t scan_ms) noexcept = 0;
};
} // namespace chino::os::kernel::io
