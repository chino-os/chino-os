//
// Chino Network
//
#pragma once
#include "NetDefs.hpp"
#include "../device/Device.hpp"

namespace Chino
{
	namespace Network
	{
		class NetworkManager
		{
		public:
			NetworkManager();

			void TryInstallNetworkDevice(ObjectPtr<Device::Device> device);
			void Test();
		};
	}
}

extern StaticHolder<Chino::Network::NetworkManager> g_NetworkMgr;
