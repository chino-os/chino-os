//
// Chino Threading
//
#pragma once
#include "../object/Object.hpp"
#include <vector>
#include <atomic>
#include "../utils.hpp"
#include "ProcessManager.hpp"
#include <optional>

namespace Chino
{
	namespace Threading
	{
		class Waitable : public Object
		{
		public:

		protected:
			void WaitOne(std::optional<std::chrono::milliseconds> timeout = std::nullopt);

			void NotifyOne();
			void NotifyAll();
		private:
			std::vector<thread_it> waitingThreads_;
		};

		class Semaphore : public Waitable
		{
		public:
			Semaphore(size_t initialCount);

			void Take(size_t count);
			bool TryTake(size_t count, std::optional<std::chrono::milliseconds> timeout = std::nullopt);
			void Give(size_t count);
		private:
			std::atomic<size_t> count_;
		};

		class Mutex : public Waitable
		{
		public:
			Mutex();

			void Take();
			void Give();
		private:
			std::atomic<bool> avail_;
		};

		class RecursiveMutex : public Waitable
		{
		public:
			RecursiveMutex();

			size_t Take();
			size_t Give();
		private:
			std::atomic<bool> avail_;
			std::atomic<Thread*> thread_;
			std::atomic<size_t> depth_;
		};

		class Event : public Waitable
		{
		public:
			Event(bool initialState = false, bool autoReset = true);

			void Wait();
			void Signal();
			void Reset();
		private:
			const bool autoReset_;
			std::atomic<bool> signaled_;
		};

		template<class T>
		class Locker;

		template<>
		class Locker<Mutex>
		{
		public:
			Locker(ObjectPtr<Mutex> mutex);
			~Locker();

			Locker(const Locker&) = delete;
			Locker& operator=(const Locker&) = delete;

			Locker(Locker&& other) = default;
			Locker& operator=(Locker&& other) = default;
		private:
			ObjectPtr<Mutex> mutex_;
		};

		class ThreadSynchronizer
		{
		public:
			ThreadSynchronizer();

		private:

		};
	}
}

extern StaticHolder<Chino::Threading::ThreadSynchronizer> g_ThreadSync;
