//
// Chino Threading
//
#pragma once
#include <libarch/arch.h>
#include "../../libchino/chino.h"
#include "../utils.hpp"
#include <list>
#include <list.hpp>
#include <string_view>
#include <string>
#include <array>
#include <atomic>
#include <queue>
#include <chrono>
#include "../object/Object.hpp"

namespace Chino
{
	namespace Threading
	{
		class Process;
		class ProcessManager;

		enum class ThreadState
		{
			Invalid,
			Ready,
			Blocked,
			Running
		};

		enum class ThreadWakeupReason
		{
			None,
			Signal,
			Timeout
		};

		class Thread : public Object, public FreeObjectAccess
		{
		public:
			Thread(ObjectPtr<Process> process, std::function<void()> threadMain, uint32_t priority, size_t stackSize);

			uint32_t GetPriority() const noexcept { return priority_; }
			ThreadContext_Arch& GetContext() noexcept { return threadContext_; }
			ObjectPtr<Process> GetProcess() noexcept { return process_; }
			ThreadWakeupReason GetWeakupReason() const noexcept { return weakupReason_; }
		private:
			static void ThreadMainThunk(Thread* thread);
		private:
			friend class ProcessManager;

			ObjectPtr<Process> process_;
			std::function<void()> threadMain_;
			ThreadContext_Arch threadContext_;
			uint32_t priority_;
			std::unique_ptr<uint8_t[]> stack_;
			ThreadState state_;
			ThreadWakeupReason weakupReason_;
			size_t delayToken_;
		};

		class Process : public Object, public FreeObjectAccess
		{
		public:
			Process(std::string_view name);

			ObjectPtr<Thread> AddThread(std::function<void()> threadMain, uint32_t priority, size_t stackSize = DEFAULT_THREAD_STACK_SIZE);
		private:
			std::string name_;
			std::vector<ObjectPtr<Thread>> threads_;
		};

		typedef Chino::list<ObjectPtr<Thread>>::iterator thread_it;

		class ProcessManager
		{
			struct DelayedEntry
			{
				size_t Tick;
				thread_it Thread;
				size_t DelayToken;

				DelayedEntry(size_t tick, thread_it thread, size_t delayToken)
					:Tick(tick), Thread(thread), DelayToken(delayToken) {}

				bool operator>(const DelayedEntry& other) const noexcept
				{
					return Tick > other.Tick;
				}
			};
		public:
			ProcessManager();

			ObjectPtr<Process> CreateProcess(std::string_view name, uint32_t mainThreadPriority, std::function<void()> threadMain, size_t mainThreadStackSize = DEFAULT_THREAD_STACK_SIZE);
			void AddReadyThread(ObjectPtr<Thread> thread);
			void StartScheduler();
			bool IncrementTick();
			void SwitchThreadContext();

			ObjectPtr<Thread> GetCurrentThread();
			thread_it DetachCurrentThread();
			void SleepCurrentThread(std::chrono::milliseconds timeout);
			void AttachReadyThread(thread_it thread);
			void DelayThread(thread_it thread, std::chrono::milliseconds timeout);
		private:
			thread_it SelectNextSwitchToThread();
		private:
			std::vector<ObjectPtr<Process>> _processes;
			std::array<Chino::list<ObjectPtr<Thread>>, MAX_THREAD_PRIORITY + 1> readyThreads_;
			std::priority_queue<DelayedEntry, std::vector<DelayedEntry>, std::greater<DelayedEntry>> delayedThreads_;
			thread_it runningThread_;
			thread_it nextThread_;
			size_t tickCount_;
			ObjectPtr<Process> idleProcess_;
		};

		class kernel_critical : private NonCopyOrMovable
		{
		public:
			kernel_critical();
			~kernel_critical();
		private:
			static std::atomic<size_t> coreTaken_;
			static std::atomic<size_t> depth_;
		};
	}
}

extern StaticHolder<Chino::Threading::ProcessManager> g_ProcessMgr;
