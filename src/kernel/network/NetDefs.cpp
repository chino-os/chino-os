//
// Chino Network
//
#include "NetDefs.hpp"
#include "../kdebug.hpp"

using namespace Chino;
using namespace Chino::Network;

const IPAddress IPAddress::IPv4Any(std::array<uint8_t, 4>{ 0, 0, 0, 0 });

SocketAddress::~SocketAddress()
{

}

AddressFamily IPEndPoint::GetAddressFamily() const noexcept
{
	return address_.GetAddressFamily();
}