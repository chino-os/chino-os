// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#include "host_ble.h"
#include <chino/os/kernel/kd.h>
#include <roapi.h>
#include <windows.devices.bluetooth.advertisement.h>
#include <wrl.h>

using namespace chino;
using namespace chino::os;
using namespace chino::os::kernel;
using namespace chino::os::kernel::io;
using namespace chino::os::drivers;
using namespace ABI::Windows::Devices::Bluetooth;
using namespace ABI::Windows::Devices::Bluetooth::Advertisement;
using namespace ABI::Windows::Foundation;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
namespace wf = Windows::Foundation;

void __cdecl operator delete(void *) {}

result<void> host_ble_device::install() noexcept { return ok(); }

result<void> host_ble_device::close(file &file) noexcept { return ok(); }

class handler
    : public RuntimeClass<
          RuntimeClassFlags<RuntimeClassType::ClassicCom>,
          ITypedEventHandler<BluetoothLEAdvertisementWatcher *, BluetoothLEAdvertisementReceivedEventArgs *>> {
  public:
    STDMETHOD_(ULONG, Release)() override {
        ULONG ref = InternalRelease();
        if (ref == 0) {
            auto modulePtr = ::Microsoft::WRL::GetModuleBase();
            if (modulePtr != nullptr) {
                modulePtr->DecrementObjectCount();
            }
        }

        return ref;
    }

    HRESULT STDMETHODCALLTYPE Invoke(_In_ IBluetoothLEAdvertisementWatcher *sender,
                                     _In_ IBluetoothLEAdvertisementReceivedEventArgs *args) noexcept override {
        ComPtr<IBluetoothLEAdvertisement> adv;
        args->get_Advertisement(&adv);
        HString local_name;
        adv->get_LocalName(local_name.GetAddressOf());
        return S_OK;
    }
};

result<void> host_ble_device::scan(ble_scan_callback_t callback, void *callback_arg, uint32_t scan_ms) noexcept {
    handler h;
    auto bleWatcherClsName =
        HString::MakeReference(RuntimeClass_Windows_Devices_Bluetooth_Advertisement_BluetoothLEAdvertisementWatcher);
    ComPtr<IBluetoothLEAdvertisementWatcher> bleWather;
    TRY_HR(wf::ActivateInstance(bleWatcherClsName.Get(), &bleWather));
    TRY_HR(bleWather->put_ScanningMode(BluetoothLEScanningMode_Active));

    EventRegistrationToken received_token;
    TRY_HR(bleWather->add_Received(&h, &received_token))
    TRY_HR(bleWather->Start());
    while (true) {
    }
    return ok();
}

result<void> host_ble_driver::install_device(host_ble_device &device) noexcept {
    try_(device.install());
    try_(io::attach_device(device));
    return ok();
}
