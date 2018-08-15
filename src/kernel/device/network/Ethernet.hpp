//
// Kernel Device
//
#pragma once
#include "../Device.hpp"

namespace Chino
{
	namespace Device
	{
		class EthernetController;

		struct INetworkHandler
		{

		};

		using MacAddress = std::array<uint8_t, 6>;

		class EthernetController : public Device
		{
		public:
			virtual void SetHandler(INetworkHandler* handler) = 0;
			virtual bool IsPacketAvailable() = 0;
			virtual void Reset(const MacAddress& macAddress) = 0;

			virtual void StartSend(size_t length) = 0;
			virtual void WriteSendBuffer(gsl::span<const uint8_t> buffer) = 0;
			virtual void CommitSend() = 0;

			virtual size_t StartReceive() = 0;
			virtual void ReadReceiveBuffer(gsl::span<uint8_t> buffer) = 0;
			virtual void FinishReceive() = 0;
		};
	}
}
