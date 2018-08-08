//
// Chino Network
//
#pragma once
#include "../object/Object.hpp"

namespace Chino
{
	namespace Network
	{
		enum class AddressFamily
		{
			Unknown,
			IPv4
		};

		enum class SocketType
		{
			Stream,
			Datagram
		};

		enum class ProtocolType
		{
			Tcp
		};

		class SocketAddress
		{
		public:
			virtual ~SocketAddress();

			virtual AddressFamily GetAddressFamily() const noexcept = 0;
		};

		struct IPAddress
		{

		};

		class IPEndPoint : public SocketAddress
		{
		public:
			IPEndPoint(IPAddress address);
		};

		enum class SocketShutdown
		{
			Send,
			Receive,
			Both
		};
	}
}
