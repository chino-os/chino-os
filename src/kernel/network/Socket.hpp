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
			~Socket();

			ObjectPtr<Socket> Accept();
			void Bind(const std::shared_ptr<const SocketAddress>& address);
			void Connect(const std::shared_ptr<const SocketAddress>& address);
			void Listen(size_t backlog);
			void Shutdown(SocketShutdown how);

			void Send(BufferList<const uint8_t> bufferList);
			size_t Receive(BufferList<uint8_t> bufferList);
		private:
			Socket(int sock);
		private:
			int sock_;
		};
	}
}
