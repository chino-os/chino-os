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

		class IPAddress
		{
		public:
			static const IPAddress IPv4Any;

			IPAddress(const std::array<uint8_t, 4> ipv4)
				:addr_(ipv4), addressFamily_(AddressFamily::IPv4) {}

			AddressFamily GetAddressFamily() const noexcept { return addressFamily_; }

			const std::array<uint8_t, 4>& GetIPv4() const
			{ 
				assert(addressFamily_ == AddressFamily::IPv4);
				return addr_; 
			}
		private:
			std::array<uint8_t, 4> addr_;
			AddressFamily addressFamily_;
		};

		class IPEndPoint : public SocketAddress
		{
		public:
			IPEndPoint(IPAddress address, uint16_t port)
				:address_(std::move(address)), port_(port) {}

			virtual AddressFamily GetAddressFamily() const noexcept override;

			const IPAddress& GetAddress() const noexcept { return address_; }
			uint16_t GetPort() const noexcept { return port_; }
		private:
			IPAddress address_;
			uint16_t port_;
		};

		enum class SocketShutdown
		{
			Send,
			Receive,
			Both
		};
	}
}
