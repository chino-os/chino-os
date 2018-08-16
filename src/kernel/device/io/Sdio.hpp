//
// Kernel Device
//
#pragma once
#include "../Device.hpp"

namespace Chino
{
	namespace Device
	{
		struct SdioCommand
		{

		};

		class SdioController : public Device
		{
		public:
			virtual ObjectPtr<Driver> TryLoadDriver() override;

			virtual void Reset() = 0;
		};
	}
}
