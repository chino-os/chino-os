//
// Kernel Device
//
#pragma once
#include "../Device.hpp"

namespace Chino
{
	namespace Device
	{
		class PicDevice : public Device
		{
		public:
			virtual size_t GetId() const noexcept = 0;

			virtual void SetIRQEnabled(int32_t irq, bool enable) = 0;
			virtual bool GetIRQPending(int32_t irq) = 0;
			virtual void SetIRQPending(int32_t irq, bool pending) = 0;
		};
	}
}
