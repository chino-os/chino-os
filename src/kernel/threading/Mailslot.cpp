//
// Chino Threading
//
#include "Mailslot.hpp"
#include "../kdebug.hpp"

using namespace Chino;
using namespace Chino::Threading;

Mailslot::Mailslot()
	:mutex_(MakeObject<Mutex>()), avail_(MakeObject<Semaphore>(0))
{

}

void Mailslot::Send(gsl::span<const uint8_t> message)
{
	auto size = size_t(message.size());

	{
		Locker<Mutex> locker(mutex_);

		// write size
		{
			auto start = reinterpret_cast<const uint8_t*>(&size);
			for (size_t i = 0; i < sizeof(size); i++)
				queue_.push_back(*start++);
		}

		// write data
		for (auto data : message)
			queue_.push_back(data);
	}

	avail_->Give(1);
}

bool Mailslot::TryReceive(size_t& messageSize, gsl::span<uint8_t> message)
{
	if (avail_->TryTake(1))
	{
		ReadMessage(messageSize, message);
		return true;
	}

	return false;
}

void Mailslot::Receive(size_t& messageSize, gsl::span<uint8_t> message)
{
	avail_->Take(1);
	ReadMessage(messageSize, message);
}

void Mailslot::ReadMessage(size_t& messageSize, gsl::span<uint8_t> message)
{
	size_t size;
	Locker<Mutex> locker(mutex_);

	// write size
	{
		auto start = reinterpret_cast<uint8_t*>(&size);
		for (size_t i = 0; i < sizeof(size); i++)
			*start++ = queue_[i];
	}

	messageSize = size;
	if (message.size() < size)
		throw std::invalid_argument("Not enough buffer.");

	for (size_t i = 0; i < sizeof(size); i++)
		queue_.pop_front();

	// write data
	for (size_t i = 0; i < size; i++)
	{
		message[i] = queue_.front();
		queue_.pop_front();
	}
}
