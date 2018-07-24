//
// Kernel Device
//
#pragma once
#include "../kernel_iface.h"
#include "../object/Object.hpp"
#include "Driver.hpp"
#include <gsl/gsl>

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

		template<class T>
		struct BufferList
		{
			gsl::span<gsl::span<T>> Buffers;

			size_t GetTotalSize() const noexcept
			{
				size_t result = 0;
				for (auto& buffer : Buffers)
					result += buffer.size();
				return result;
			}
		};

		class Device : public Object, public virtual IObjectAccess
		{
		public:
			virtual ObjectPtr<Driver> TryLoadDriver();
			virtual DeviceType GetType() const noexcept;
		};
	}
}
