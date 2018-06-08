//
// Chino Thread
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
#include "../object/Object.hpp"

namespace Chino
{
	namespace Thread
	{
		class Thread : public Object, public FreeObjectAccess
		{
		public:
			Thread(std::function<void()> threadMain, uint32_t priority);

			uint32_t GetPriority() const noexcept { return priority_; }
			ThreadContext_Arch& GetContext() noexcept { return threadContext_; }
		private:
			static void ThreadMainThunk(Thread* thread);
		private:
			std::function<void()> threadMain_;
			ThreadContext_Arch threadContext_;
			uint32_t priority_;
			std::unique_ptr<uint8_t[]> stack_;
		};

		class Process : public Object, public FreeObjectAccess
		{
		public:
			Process(std::string_view name);

			Thread& AddThread(std::function<void()> threadMain, uint32_t priority);
		private:
			std::string name_;
			std::vector<ObjectPtr<Thread>> threads_;
		};

		class ProcessManager
		{
			typedef Chino::list<ObjectPtr<Thread>>::iterator thread_it;
		public:
			ProcessManager();

			Process& CreateProcess(std::string_view name, uint32_t mainThreadPriority, std::function<void()> threadMain);
			void AddReadyThread(Thread& thread);
			void StartScheduler();
			ThreadContext_Arch& SwitchThreadContext();
		private:
			thread_it SelectNextSwitchToThread();
		private:
			std::vector<ObjectPtr<Process>> _processes;
			std::array<Chino::list<ObjectPtr<Thread>>, MAX_THREAD_PRIORITY + 1> readyThreads_;
			thread_it runningThread_;
			ObjectPtr<Process> idleProcess_;
		};
	}
}

extern StaticHolder<Chino::Thread::ProcessManager> g_ProcessMgr;