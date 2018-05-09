//
// Kernel Device
//
#pragma once
#include "../../Driver.hpp"
#include "../Drive.hpp"

#include <cstdint>
#include <cstddef>
#include <memory>

#define DECLARE_PARTITION_DRIVER(Type) \
static const Chino::Device::PartitionDriverDescriptor Descriptor; \
static std::unique_ptr<Chino::Device::Driver> Activate(Chino::Device::Partition& device); \
static bool IsSupported(Chino::Device::Partition& device);

#define DEFINE_PARTITION_DRIVER_DESC(Type) \
std::unique_ptr<Chino::Device::Driver> Type::Activate(Chino::Device::Partition& device) \
{ return std::make_unique<Type>(device); } \
const Chino::Device::PartitionDriverDescriptor Type::Descriptor = { Type::Activate, Type::IsSupported };

namespace Chino
{
	namespace Device
	{
		class Partition;
		typedef std::unique_ptr<Driver>(*PartitionDriverActivator_t)(Partition& device);
		typedef bool(*PartitionDriverIsSupported_t)(Partition& device);

		struct PartitionDriverDescriptor
		{
			PartitionDriverActivator_t Activator;
			PartitionDriverIsSupported_t IsSupported;
		};

		class Partition
		{
		public:
			Partition(DriveDevice& drive, size_t startLBA);
			std::unique_ptr<Driver> TryLoadDriver();

			void Read(uint64_t lba, size_t count, uint8_t* buffer);

			size_t BlockSize;
			size_t MaxLBA;
		private:
			DriveDevice& drive_;
			size_t startLBA_;
		};

		extern const PartitionDriverDescriptor* g_PartitionDrivers[];
	}
}