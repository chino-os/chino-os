//
// Kernel Device
//
#pragma once
#include "../kernel_iface.h"

namespace Chino
{
	namespace Device
	{
		class Driver
		{
		public:
			virtual void Install() = 0;
		};
	}
}