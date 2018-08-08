//
// Kernel Device
//
#pragma once
#include "../kernel_iface.h"
#include "../object/Object.hpp"
#include "Driver.hpp"
#include "../utility/BufferList.hpp"

namespace Chino
{
	namespace Device
	{
		enum class DeviceType
		{
			Storage,
			Partition,
			Other
		};

		enum class TransmitRole
		{
			Sender,
			Receiver
		};

		class Device : public Object, public virtual IObjectAccess
		{
		public:
			virtual ObjectPtr<Driver> TryLoadDriver();
			virtual DeviceType GetType() const noexcept;
		};
	}
}
