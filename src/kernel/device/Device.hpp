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

		class Device : public Object
		{
		public:
			virtual void SetIsEnabled(bool enabled);
			virtual ObjectPtr<Driver> TryLoadDriver();
			virtual DeviceType GetType() const noexcept;
		};
	}
}
