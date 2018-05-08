//
// Kernel Device
//
#pragma once
#include "../Driver.hpp"

#include <cstdint>
#include <cstddef>
#include <memory>

#define DECLARE_DRIVE_DRIVER(Type) \
static const Chino::Device::DriveDriverDescriptor Descriptor; \
static std::unique_ptr<Chino::Device::Driver> Activate(const Chino::Device::DriveDevice& device); \
static bool IsSupported(const Chino::Device::DriveDevice& device);

#define DEFINE_DRIVE_DRIVER_DESC(Type) \
std::unique_ptr<Chino::Device::Driver> Type::Activate(const Chino::Device::DriveDevice& device) \
{ return std::make_unique<Type>(device); } \
const Chino::Device::DriveDriverDescriptor Type::Descriptor = { Type::Activate, Type::IsSupported };

namespace Chino
{
	namespace Device
	{
		class DriveDevice;
		typedef std::unique_ptr<Driver>(*DriveDriverActivator_t)(const DriveDevice& device);
		typedef bool(*DriveDriverIsSupported_t)(const DriveDevice& device);

		struct DriveDriverDescriptor
		{
			DriveDriverActivator_t Activator;
			DriveDriverIsSupported_t IsSupported;
		};

		class DriveDevice
		{
		public:
			std::unique_ptr<Driver> TryLoadDriver();

			virtual void Read(uint64_t lba, size_t count, uint8_t* buffer) = 0;

			size_t BlockSize;
			size_t MaxLBA;
		};

		extern const DriveDriverDescriptor* g_DriveDrivers[];
	}
}
