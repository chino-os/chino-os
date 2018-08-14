//
// Chino Network
//
#pragma once
#include "NetDefs.hpp"
#include "../device/network/Ethernet.hpp"

namespace Chino
{
	namespace Network
	{
		struct NetworkInterface : public Object
		{

		};

		class NetworkManager
		{
		public:
			NetworkManager();

			ObjectPtr<NetworkInterface> InstallNetworkDevice(ObjectAccessor<Device::EthernetController> device);
			void Test();
		private:
			std::vector<ObjectPtr<NetworkInterface>> netifs_;
		};
	}
}

extern StaticHolder<Chino::Network::NetworkManager> g_NetworkMgr;
