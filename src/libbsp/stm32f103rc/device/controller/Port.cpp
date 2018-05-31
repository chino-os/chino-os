//
// Kernel Device
//
#include "Port.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>

using namespace Chino;
using namespace Chino::Device;

PortDevice::PortDevice(const FDTDevice& fdt)
{
	auto regProp = fdt.GetProperty("reg");
	kassert(regProp.has_value());
	regAddr_ = regProp->GetUInt32(0);

	g_ObjectMgr->GetDirectory(WellKnownDirectory::Device).AddItem("rcc", *this);
}
