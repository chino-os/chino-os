//
// Kernel Device
//
#include "Fdt.hpp"
#include <kernel/kdebug.hpp>
#include <libfdt/libfdt.h>

using namespace Chino::Device;

FDTDevice::FDTDevice(const void* fdt, int node, int depth)
	:fdt_(fdt), node_(node)
{
#if 1
	while (depth--)
		g_Logger->PutChar(' ');
	auto name = fdt_get_name(fdt, node, NULL);
	g_Logger->PutFormat("Node(%d): %s", node, name);
	auto device_type = fdt_get_property(fdt, node, "device_type", NULL);
	if (device_type)
		g_Logger->PutFormat(", type: %s", device_type->data);
	auto compatible = fdt_get_property(fdt, node, "compatible", NULL);
	if (compatible)
		g_Logger->PutFormat(", compatible: %s", compatible->data);
	g_Logger->PutChar('\n');
#endif
}

bool FDTDevice::HasDeviceType(std::string_view deviceType) const noexcept
{
	auto prop = fdt_get_property(fdt_, node_, "device_type", NULL);
	if (prop)
		return deviceType.compare(prop->data) == 0;
	return false;
}

bool FDTDevice::HasCompatible(std::string_view compatible) const noexcept
{
	auto prop = fdt_get_property(fdt_, node_, "compatible", NULL);
	if (prop)
		return compatible.compare(prop->data) == 0;
	return false;
}

std::unique_ptr<Driver> FDTDevice::TryLoadDriver()
{
	auto head = g_FDTDrivers;
	auto cnt = *head;

	while (cnt)
	{
		if (cnt->IsSupported(*this))
			return cnt->Activator(*this);

		cnt = *++head;
	}

	return nullptr;
}
