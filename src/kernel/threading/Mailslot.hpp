//
// Chino Threading
//
#pragma once
#include "../object/Object.hpp"
#include <queue>
#include "ThreadSynchronizer.hpp"
#include <gsl/gsl>

namespace Chino
{
	namespace Threading
	{
		class Mailslot : public Object
		{
		public:
			Mailslot();

			void Send(gsl::span<const uint8_t> message);
			bool TryReceive(size_t& messageSize, gsl::span<uint8_t> message, std::optional<std::chrono::milliseconds> timeout = std::nullopt);
			void Receive(size_t& messageSize, gsl::span<uint8_t> message);
		private:
			void ReadMessage(size_t& messageSize, gsl::span<uint8_t> message);
		private:
			ObjectPtr<Mutex> mutex_;
			ObjectPtr<Semaphore> avail_;
			std::deque<uint8_t> queue_;
		};
	}
}
