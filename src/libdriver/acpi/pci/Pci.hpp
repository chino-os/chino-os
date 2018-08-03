//
// Kernel Device
//
#pragma once

extern "C"
{
#include <efi.h>
#include <efilink.h>
#include <pci22.h>
}

#include <kernel/device/Device.hpp>
#include <cstddef>
#include <memory>
#include <array>

#define DECLARE_PCI_DRIVER(Type) \
static const Chino::Device::PCIDriverDescriptor Descriptor; \
static Chino::ObjectPtr<Chino::Device::Driver> Activate(const Chino::Device::PCIDevice& device); \
static bool IsSupported(const Chino::Device::PCIDevice& device);

#define DEFINE_PCI_DRIVER_DESC(Type, ClassCode, SubClass) \
Chino::ObjectPtr<Chino::Device::Driver> Type::Activate(const Chino::Device::PCIDevice& device) \
{ return Chino::MakeObject<Type>(device); } \
const Chino::Device::PCIDriverDescriptor Type::Descriptor = { { SubClass, ClassCode }, Type::Activate, Type::IsSupported };

namespace Chino
{
	namespace Device
	{
		class PCIDevice;
		typedef Chino::ObjectPtr<Driver> (*PCIDriverActivator_t)(const PCIDevice& device);
		typedef bool (*PCIDriverIsSupported_t)(const PCIDevice& device);

		struct PCIDriverDescriptor
		{
			std::array<uint8_t, 2> ClassCode;
			PCIDriverActivator_t Activator;
			PCIDriverIsSupported_t IsSupported;
		};

		class PCIDevice : public Device, public ExclusiveObjectAccess
		{
		public:
			PCIDevice(size_t bus, size_t device, size_t function, PCI_DEVICE_INDEPENDENT_REGION* configuration);

			Chino::ObjectPtr<Driver> TryLoadDriver();

			volatile PCI_DEVICE_INDEPENDENT_REGION* GetConfigurationSpace() const noexcept { return config_; }
		private:
			volatile PCI_DEVICE_INDEPENDENT_REGION * config_;
		};

		extern const PCIDriverDescriptor* g_PCIDrivers[];
	}
}
