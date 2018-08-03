//
// Chino Threading
//
#include "ThreadSynchronizer.hpp"
#include <kernel/kdebug.hpp>

using namespace Chino;
using namespace Chino::Threading;

void Waitable::WaitOne()
{
	auto it = g_ProcessMgr->DetachCurrentThread();
	if (std::find(waitingThreads_.begin(), waitingThreads_.end(), it) == waitingThreads_.end())
	{
		waitingThreads_.emplace_back(it);
	}
}

void Waitable::NotifyOne()
{
	if (!waitingThreads_.empty())
	{
		auto thread = waitingThreads_.back();
		waitingThreads_.pop_back();

		g_ProcessMgr->AttachReadyThread(thread);
	}
}

void Waitable::NotifyAll()
{
	while (!waitingThreads_.empty())
	{
		auto thread = waitingThreads_.back();
		waitingThreads_.pop_back();

		g_ProcessMgr->AttachReadyThread(thread);
	}
}

Semaphore::Semaphore(size_t initialCount)
	:count_(initialCount)
{

}

void Semaphore::Take(size_t count)
{
	while (count)
	{
		kernel_critical kc;
		auto expected = count_.load(std::memory_order_relaxed);
		if (expected < count)
		{
			WaitOne();
		}
		else
		{
			if (count_.compare_exchange_strong(expected, expected - count, std::memory_order_relaxed))
				break;
		}
	}
}

void Semaphore::Give(size_t count)
{
	if (count)
	{
		kernel_critical kc;
		count_.fetch_add(count, std::memory_order_relaxed);
		NotifyAll();
	}
}

Mutex::Mutex()
	:avail_(true)
{

}

void Mutex::Take()
{
	while (true)
	{
		kernel_critical kc;
		auto expected = avail_.load(std::memory_order_relaxed);
		if (!expected)
		{
			WaitOne();
		}
		else
		{
			if (avail_.compare_exchange_strong(expected, false, std::memory_order_relaxed))
				break;
		}
	}
}

void Mutex::Give()
{
	kernel_critical kc;
	avail_.store(true, std::memory_order_relaxed);
	NotifyOne();
}

Event::Event(bool initialState, bool autoReset)
	:autoReset_(autoReset), signaled_(initialState)
{

}

void Event::Wait()
{
	while (true)
	{
		kernel_critical kc;
		auto expected = signaled_.load(std::memory_order_acquire);
		if (!expected)
		{
			WaitOne();
		}
		else
		{
			if (autoReset_)
			{
				if (signaled_.compare_exchange_strong(expected, false, std::memory_order_acq_rel))
					break;
			}
			else
			{
				break;
			}
		}
	}
}

void Event::Signal()
{
	kernel_critical kc;
	signaled_.store(true, std::memory_order_release);
	NotifyAll();
}

void Event::Reset()
{
	signaled_.store(false, std::memory_order_relaxed);
}

Locker<Mutex>::Locker(ObjectPtr<Mutex> mutex)
	:mutex_(std::move(mutex))
{
	mutex_->Take();
}

Locker<Mutex>::~Locker()
{
	if (mutex_)
		mutex_->Give();
}
