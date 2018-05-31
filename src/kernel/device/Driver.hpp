//
// Kernel Device
//
#pragma once
#include "../kernel_iface.h"
#include "../object/Object.hpp"

namespace Chino
{
	namespace Device
	{
		class Driver : public Object, public ExclusiveObjectAccess
		{
		public:
			virtual void Install() = 0;
		};
	}
}
