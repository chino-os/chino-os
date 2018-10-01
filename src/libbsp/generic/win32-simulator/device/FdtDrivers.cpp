//
// Kernel Device
//
#include <libdriver/devicetree/Fdt.hpp>
#include <Windows.h>
#include "../devicetree/resource.h"

#include <libdriver/devicetree/simulator/display/lcd/BasicDisplay.hpp>
#include <libdriver/devicetree/simulator/storage/SDStorage.hpp>

using namespace Chino::Device;

struct dtb_resource
{
	gsl::span<const uint8_t> Data;

	dtb_resource()
	{
		auto hres = FindResource(nullptr, MAKEINTRESOURCE(IDR_DTB), L"Binary");
		auto size = SizeofResource(nullptr, hres);
		auto hmem = LoadResource(nullptr, hres);
		auto data = LockResource(hmem);
		Data = { reinterpret_cast<const uint8_t*>(data), size };
	}
};

gsl::span<const uint8_t> Chino::Device::BSPGetFdtData() noexcept
{
	static dtb_resource dtb;
	return dtb.Data;
}

#define REF_FDT_DRIVER_DESC(Type) &Type::Descriptor

const FDTDriverDescriptor* Chino::Device::g_FDTDrivers[] =
{
    REF_FDT_DRIVER_DESC(BasicDisplayDriver),
    REF_FDT_DRIVER_DESC(SDStorageDriver),
	nullptr
};
