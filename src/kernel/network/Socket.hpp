//
// Chino Network
//
#pragma once
#include "NetDefs.hpp"
#include "../utility/BufferList.hpp"

namespace Chino
{
	namespace Network
	{
		class Socket : public Object
		{
		public:
			Socket(AddressFamily addressFamily, SocketType socketType, ProtocolType protocolType);

			ObjectPtr<Socket> Accept();
			void Bind(const std::shared_ptr<SocketAddress>& address);
			void Connect(const std::shared_ptr<SocketAddress>& address);
			void Listen(size_t backlog);
			void Shutdown(SocketShutdown how);

			void Send(BufferList<const uint8_t> buffer);
			size_t Receive(BufferList<uint8_t> buffer);
		};
	}
}
