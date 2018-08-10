//
// Kernel Device
//
#pragma once
#include "../Device.hpp"

namespace Chino
{
	namespace Device
	{
		class EthernetController;

		struct INetworkHandler
		{

		};

		class EthernetController : public Device
		{
		public:
			virtual void SetHandler(INetworkHandler* handler) = 0;
			virtual bool IsPacketAvailable() = 0;
			//virtual void 
		};
	}
}
