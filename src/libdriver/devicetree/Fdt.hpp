//
// Kernel Device
//
#pragma once

#include <kernel/device/Device.hpp>
#include <cstddef>
#include <memory>
#include <array>
#include <string_view>
#include <optional>

#define DECLARE_FDT_DRIVER(Type) \
Type(const Chino::Device::FDTDevice& device); \
static const Chino::Device::FDTDriverDescriptor Descriptor; \
static Chino::ObjectPtr<Chino::Device::Driver> Activate(const Chino::Device::FDTDevice& device); \
static bool IsSupported(const Chino::Device::FDTDevice& device);

#define DEFINE_FDT_DRIVER_DESC(Type) \
Chino::ObjectPtr<Chino::Device::Driver> Type::Activate(const Chino::Device::FDTDevice& device) \
{ return Chino::MakeObject<Type>(device); } \
const Chino::Device::FDTDriverDescriptor Type::Descriptor = { Type::Activate, Type::IsSupported };

#define DEFINE_FDT_DRIVER_DESC_1(Type, DeviceType, Compatible) \
Chino::ObjectPtr<Chino::Device::Driver> Type::Activate(const Chino::Device::FDTDevice& device) \
{ return Chino::MakeObject<Type>(device); } \
const Chino::Device::FDTDriverDescriptor Type::Descriptor = { Type::Activate, Type::IsSupported }; \
bool Type::IsSupported(const Chino::Device::FDTDevice& device) \
{ \
return device.HasDeviceType(DeviceType) && device.HasCompatible(Compatible); \
}

namespace Chino
{
	namespace Device
	{
		class FDTDevice;
		typedef Chino::ObjectPtr<Driver>(*PFDTDriverActivator_t)(const FDTDevice& device);
		typedef bool(*FDTDriverIsSupported_t)(const FDTDevice& device);

		struct FDTDriverDescriptor
		{
			PFDTDriverActivator_t Activator;
			FDTDriverIsSupported_t IsSupported;
		};

		struct FDTProperty
		{
			uint32_t nameOffset;
			const void* data;
			size_t length;

			uint32_t GetUInt32(size_t index) const noexcept;
			std::string_view GetString() const noexcept;
		};

		class FDTDevice : public Device, public ExclusiveObjectAccess
		{
		public:
			FDTDevice(const void* fdt, int node, int depth);

			bool HasDeviceType(std::string_view deviceType) const noexcept;
			bool HasCompatible(std::string_view compatible) const noexcept;

			std::string_view GetName() const noexcept;

			std::optional<FDTProperty> GetProperty(std::string_view name) const noexcept;
			std::optional<FDTProperty> GetPropertyOrInherited(std::string_view name) const noexcept;

			virtual ObjectPtr<Driver> TryLoadDriver() override;
			virtual DeviceType GetType() const noexcept override;
		private:
			const void* fdt_;
			int node_;
		};

		extern const FDTDriverDescriptor* g_FDTDrivers[];
	}
}
