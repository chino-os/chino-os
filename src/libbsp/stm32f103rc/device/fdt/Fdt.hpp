//
// Kernel Device
//
#pragma once

#include <kernel/device/Driver.hpp>
#include <cstddef>
#include <memory>
#include <array>
#include <string_view>

#define DECLARE_FDT_DRIVER(Type) \
static const Chino::Device::FDTDriverDescriptor Descriptor; \
static std::unique_ptr<Chino::Device::Driver> Activate(const Chino::Device::FDTDevice& device); \
static bool IsSupported(const Chino::Device::FDTDevice& device);

#define DEFINE_PCI_DRIVER_DESC(Type, ClassCode, SubClass) \
std::unique_ptr<Chino::Device::Driver> Type::Activate(const Chino::Device::FDTDevice& device) \
{ return std::make_unique<Type>(device); } \
const Chino::Device::FDTDriverDescriptor Type::Descriptor = { Type::Activate, Type::IsSupported };

namespace Chino
{
	namespace Device
	{
		class FDTDevice;
		typedef std::unique_ptr<Driver>(*PFDTDriverActivator_t)(const FDTDevice& device);
		typedef bool(*FDTDriverIsSupported_t)(const FDTDevice& device);

		struct FDTDriverDescriptor
		{
			PFDTDriverActivator_t Activator;
			FDTDriverIsSupported_t IsSupported;
		};

		class FDTDevice
		{
		public:
			FDTDevice(const void* fdt, int node, int depth);

			std::unique_ptr<Driver> TryLoadDriver();
		private:
			const void* fdt_;
			int node_;
		};

		extern const FDTDriverDescriptor* g_FDTDrivers[];
	}
}
