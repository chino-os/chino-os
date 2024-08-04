// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "../../../hal/archs/emulator/emulator.h"
#include "../../ke/ke_services.h"
#include "../../ps/task/process.h"
#include "../io_manager.h"
#include <bluetoothleapis.h>
#include <chino/os/kernel/kd.h>
#include <chino/os/kernel/ob.h>
#include <roapi.h>
#include <windows.devices.bluetooth.advertisement.h>

#define __WRL_ASSERT__(x) kassert(x)
#include <wrl.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;
using namespace std::string_view_literals;

result<void> io::initialize_ble_manager() noexcept {
    Windows::Foundation::Initialize(RO_INIT_MULTITHREADED);
    auto bleWatcherClsName = Microsoft::WRL::Wrappers::HString::MakeReference(
        RuntimeClass_Windows_Devices_Bluetooth_Advertisement_BluetoothLEAdvertisementWatcher);
    Microsoft::WRL::ComPtr<IInspectable> bleWatherInsp;
    RoActivateInstance(bleWatcherClsName.Get(), bleWatherInsp.GetAddressOf());
    return ok();
}
