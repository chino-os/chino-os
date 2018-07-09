//
// Kernel Device
//
#pragma once
#include "../kernel_iface.h"
#include "../object/Object.hpp"
#include "Driver.hpp"

namespace Chino
{
	namespace Device
	{
		enum class DeviceType
		{
			Drive,
			Partition,
			Other
		};

		enum class TransmitRole
		{
			Sender,
			Receiver
		};

		class Device : public Object
		{
		public:
			virtual ObjectPtr<Driver> TryLoadDriver();
			virtual DeviceType GetType() const noexcept;
		};
	}
}
