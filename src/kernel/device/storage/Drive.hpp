//
// Kernel Device
//
#pragma once
#include "../Device.hpp"

#include <cstdint>
#include <cstddef>
#include <memory>

#define DECLARE_DRIVE_DRIVER(Type) \
static const Chino::Device::DriveDriverDescriptor Descriptor; \
static Chino::ObjectPtr<Chino::Device::Driver> Activate(Chino::Device::DriveDevice& device); \
static bool IsSupported(Chino::Device::DriveDevice& device);

#define DEFINE_DRIVE_DRIVER_DESC(Type) \
Chino::ObjectPtr<Chino::Device::Driver> Type::Activate(Chino::Device::DriveDevice& device) \
{ return Chino::MakeObject<Type>(device); } \
const Chino::Device::DriveDriverDescriptor Type::Descriptor = { Type::Activate, Type::IsSupported };

namespace Chino
{
	namespace Device
	{
		class DriveDevice;
		typedef Chino::ObjectPtr<Driver>(*DriveDriverActivator_t)(DriveDevice& device);
		typedef bool(*DriveDriverIsSupported_t)(DriveDevice& device);

		struct DriveDriverDescriptor
		{
			DriveDriverActivator_t Activator;
			DriveDriverIsSupported_t IsSupported;
		};

		class DriveDevice : public Device
		{
		public:
			virtual ObjectPtr<Driver> TryLoadDriver() override;
			virtual DeviceType GetType() const noexcept override;

			virtual void Read(uint64_t lba, size_t count, uint8_t* buffer) = 0;

			size_t BlockSize;
			size_t MaxLBA;
		};

		extern const DriveDriverDescriptor* g_DriveDrivers[];
	}
}
