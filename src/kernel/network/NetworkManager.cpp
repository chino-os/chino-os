//
// Chino Network
//
#include "NetworkManager.hpp"
#include "../kdebug.hpp"
#include <lwip/init.h>

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Network;

NetworkManager::NetworkManager()
{
	lwip_init();
}

void NetworkManager::TryInstallNetworkDevice(ObjectPtr<Device::Device> device)
{
	auto type = device->GetType();
	if (type == DeviceType::NetworkInterface)
	{

	}
}
