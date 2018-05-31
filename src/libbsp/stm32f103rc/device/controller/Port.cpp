//
// Kernel Device
//
#include "Port.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <string>

using namespace Chino;
using namespace Chino::Device;

PortPin::PortPin(size_t portIndex, uintptr_t portRegAddr, PortPins pin)
	:portRegAddr_(portRegAddr), pin_(pin)
{
	static char portName[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G' };

	kassert(portIndex < std::size(portName));
	auto name = std::string("Port") + portName[portIndex] + std::to_string(size_t(pin));
	g_ObjectMgr->GetDirectory(WKD_Device).AddItem(name, *this);
}

void PortPin::SetMode(PortInputMode mode)
{

}

void PortPin::SetMode(PortOutputMode mode, PortSpeed speed)
{

}
