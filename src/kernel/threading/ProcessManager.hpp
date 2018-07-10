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
#include "../object/Object.hpp"

namespace Chino
{
	namespace Threading
	{
		class Process;

		class Thread : public Object, public FreeObjectAccess
		{
		public:
			Thread(ObjectPtr<Process> process, std::function<void()> threadMain, uint32_t priority);

			uint32_t GetPriority() const noexcept { return priority_; }
			ThreadContext_Arch& GetContext() noexcept { return threadContext_; }
			ObjectPtr<Process> GetProcess() noexcept { return process_; }
		private:
			static void ThreadMainThunk(Thread* thread);
		private:
			ObjectPtr<Process> process_;
			std::function<void()> threadMain_;
			ThreadContext_Arch threadContext_;
			uint32_t priority_;
			std::unique_ptr<uint8_t[]> stack_;
		};

		class Process : public Object, public FreeObjectAccess
		{
		public:
			Process(std::string_view name);

			ObjectPtr<Thread> AddThread(std::function<void()> threadMain, uint32_t priority);
		private:
			std::string name_;
			std::vector<ObjectPtr<Thread>> threads_;
		};

		typedef Chino::list<ObjectPtr<Thread>>::iterator thread_it;

		class ProcessManager
		{
		public:
			ProcessManager();

			ObjectPtr<Process> CreateProcess(std::string_view name, uint32_t mainThreadPriority, std::function<void()> threadMain);
			void AddReadyThread(ObjectPtr<Thread> thread);
			void StartScheduler();
			bool IncrementTick();
			void SwitchThreadContext();

			ObjectPtr<Thread> GetCurrentThread();
			thread_it DetachCurrentThread();
			void AttachReadyThread(thread_it thread);
		private:
			thread_it SelectNextSwitchToThread();
		private:
			std::vector<ObjectPtr<Process>> _processes;
			std::array<Chino::list<ObjectPtr<Thread>>, MAX_THREAD_PRIORITY + 1> readyThreads_;
			thread_it runningThread_;
			thread_it nextThread_;
			size_t tickCount_;
			ObjectPtr<Process> idleProcess_;
		};

		class kernel_critical
		{
		public:
			kernel_critical();
			~kernel_critical();
		private:
			static std::atomic<size_t> coreTaken_;
			static size_t depth_;
		};
	}
}

extern StaticHolder<Chino::Threading::ProcessManager> g_ProcessMgr;
