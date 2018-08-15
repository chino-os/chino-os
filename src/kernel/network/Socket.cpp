//
// Chino Network
//
#include "Socket.hpp"
#include "../kdebug.hpp"
#include <lwip/sockets.h>

using namespace Chino;
using namespace Chino::Network;

static void CheckLwipError(int result)
{
	if (result < 0)
		throw std::runtime_error(strerror(errno));
}

static void ToLwipSockAddr(sockaddr_in& addr, const IPEndPoint& endPoint)
{
	auto ipv4 = endPoint.GetAddress().GetIPv4();

	addr.sin_len = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(endPoint.GetPort());
	addr.sin_addr.s_addr = LWIP_MAKEU32(ipv4[0], ipv4[1], ipv4[2], ipv4[3]);
}

Socket::Socket(AddressFamily addressFamily, SocketType socketType, ProtocolType protocolType)
	:sock_(0)
{
	int domain;
	switch (addressFamily)
	{
	case AddressFamily::Unknown:
	case AddressFamily::IPv4:
		domain = AF_INET;
		break;
	default:
		throw std::invalid_argument("Invalid address family.");
	}

	int type;
	switch (socketType)
	{
	case SocketType::Stream:
		type = SOCK_STREAM;
		break;
	case SocketType::Datagram:
		type = SOCK_DGRAM;
		break;
	default:
		throw std::invalid_argument("Invalid socket type.");
	}

	int protocol;
	switch (protocolType)
	{
	case ProtocolType::Tcp:
		protocol = IPPROTO_TCP;
		break;
	default:
		throw std::invalid_argument("Invalid protocol type.");
	}

	auto sock = lwip_socket(domain, type, protocol);
	CheckLwipError(sock);
	sock_ = sock;
}

Socket::Socket(int sock)
	:sock_(sock)
{
	kassert(sock);
}

Socket::~Socket()
{
	if (sock_)
	{
		lwip_close(sock_);
		sock_ = 0;
	}
}

ObjectPtr<Socket> Socket::Accept()
{
	sockaddr_in remote;
	socklen_t remoteLen = sizeof(remote);

	auto sock = lwip_accept(sock_, reinterpret_cast<sockaddr*>(&remote), &remoteLen);
	CheckLwipError(sock);

	auto newSocket = new (std::nothrow) Socket(sock);
	if (!newSocket)
	{
		lwip_close(sock);
		throw std::bad_alloc();
	}
	else
	{
		return { std::in_place, newSocket };
	}
}

void Socket::Bind(const std::shared_ptr<const SocketAddress>& address)
{
	auto sockAddr = dynamic_cast<const IPEndPoint*>(address.get());
	if (!sockAddr) throw std::invalid_argument("Invalid address.");

	sockaddr_in addr;
	ToLwipSockAddr(addr, *sockAddr);
	CheckLwipError(lwip_bind(sock_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)));
}

void Socket::Listen(size_t backlog)
{
	CheckLwipError(lwip_listen(sock_, static_cast<int>(backlog)));
}

void Socket::Send(BufferList<const uint8_t> bufferList)
{
	for (auto buffer : bufferList.Buffers)
	{
		auto ret = lwip_write(sock_, buffer.data(), buffer.size_bytes());
		CheckLwipError(ret);
		kassert(ret == buffer.size_bytes());
	}
}

size_t Socket::Receive(BufferList<uint8_t> bufferList)
{
	size_t read = 0;
	for (auto buffer : bufferList.Buffers)
	{
		auto ret = lwip_read(sock_, buffer.data(), buffer.size_bytes());
		CheckLwipError(ret);
		if (ret != buffer.size_bytes())break;
	}

	return read;
}

void Socket::Connect(const std::shared_ptr<const SocketAddress>& address)
{
	auto sockAddr = dynamic_cast<const IPEndPoint*>(address.get());
	if (!sockAddr) throw std::invalid_argument("Invalid address.");

	sockaddr_in addr;
	ToLwipSockAddr(addr, *sockAddr);
	CheckLwipError(lwip_connect(sock_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)));
}

void Socket::Shutdown(SocketShutdown how)
{
	int innerHow;
	switch (how)
	{
	case SocketShutdown::Send:
		innerHow = SHUT_WR;
		break;
	case SocketShutdown::Receive:
		innerHow = SHUT_RD;
		break;
	case SocketShutdown::Both:
		innerHow = SHUT_RDWR;
		break;
	default:
		throw std::invalid_argument("Invalid how.");
	}

	CheckLwipError(lwip_shutdown(sock_, innerHow));
}
