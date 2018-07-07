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

Chino::ObjectPtr<Driver> FDTDevice::TryLoadDriver()
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

DeviceType FDTDevice::GetType() const noexcept
{
	return DeviceType::Other;
}

std::optional<FDTProperty> FDTDevice::GetProperty(std::string_view name) const noexcept
{
	auto prop = fdt_get_property_namelen(fdt_, node_, name.data(), name.length(), NULL);
	if (prop)
		return FDTProperty { prop->nameoff, prop->data, prop->len };
	return {};
}

std::optional<FDTProperty> FDTDevice::GetPropertyOrInherited(std::string_view name) const noexcept
{
	int cntNode = node_;

	while (cntNode > 0)
	{
		auto prop = fdt_get_property_namelen(fdt_, cntNode, name.data(), name.length(), NULL);
		if (prop)
			return FDTProperty { prop->nameoff, prop->data, prop->len };
		cntNode = fdt_parent_offset(fdt_, cntNode);
	}

	return {};
}

std::string_view FDTDevice::GetName() const noexcept
{
	int len;
	auto name = fdt_get_name(fdt_, node_, &len);
	return { name, size_t(len) };
}

uint32_t FDTProperty::GetUInt32(size_t index) const noexcept
{
	auto uiData = reinterpret_cast<const fdt32_t*>(data);
	return fdt32_to_cpu(uiData[index]);
}
