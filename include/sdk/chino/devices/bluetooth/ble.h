// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include "../../compiler.h"
#include "../../os/objectapi.h"
#include "../../result.h"
#include <array>

namespace chino::devices::bluetooth {
// @brief Bluetooth address
using bt_address = std::array<std::byte, 6>;

// @brief Address types
enum class ble_address_type {
    public_ = 0,     // Public address
    random = 1,      // Random address,
    unspecified = 2, // Unspecified address
};

// @brief Specifies the different types of Bluetooth LE advertisement payloads.
enum class ble_advertisement_type {
    // @brief The advertisement is directed and indicates that the device is connectable but not scannable. This
    // advertisement type cannot carry data.
    // This corresponds with the ADV_DIRECT_IND type defined in the Bluetooth LE specifications.
    connectable_directed = 1,

    // @brief The advertisement is undirected and indicates that the device is connectable and scannable. This
    // advertisement type can carry data.
    // This corresponds with the ADV_IND type defined in the Bluetooth LE specifications.
    connectable_undirected = 0,

    // @brief This advertisement is a 5.0 extended advertisement. This advertisement type may have different properties,
    // and is not necessarily directed, connected, scannable, nor a scan response.
    extented = 5,

    // @brief The advertisement is undirected and indicates that the device is not connectable nor scannable. This
    // advertisement type can carry data.
    // This corresponds with the ADV_NONCONN_IND type defined in the Bluetooth LE specifications.
    nonconnectable_undirected = 3,

    // @brief The advertisement is undirected and indicates that the device is scannable but not connectable. This
    // advertisement type can carry data.
    // This corresponds with the ADV_SCAN_IND type defined in the Bluetooth LE specifications.
    scannable_undirected = 2,

    // @brief This advertisement is a scan response to a scan request issued for a scannable advertisement. This
    // advertisement type can carry data.
    // This corresponds with the SCAN_RSP type defined in the Bluetooth LE specifications.
    scan_response = 4,
};

struct ble_advertisement {};

struct ble_advertisement_received {
    bt_address address;
    ble_address_type address_type;
    ble_advertisement advertisement;
};

typedef result<bool> (*ble_advertisement_callback_t)(ble_advertisement_received *, void *);
} // namespace chino::devices::bluetooth
