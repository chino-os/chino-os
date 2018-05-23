//
// Chino Thread
//
#pragma once
#include <libarch/arch.h>
#include "../../libchino/chino.h"
#include "../utils.hpp"
#include <list>
#include <common/list.hpp>
#include <string_view>
#include <string>
#include <array>

namespace Chino
{
	namespace Thread
	{
		class ProcessManager
		{
			class Thread
			{
			public:
				Thread(ThreadMain_t entryPoint, uint32_t priority, uintptr_t parameter);

				uint32_t GetPriority() const noexcept { return priority_; }
				ThreadContext_Arch& GetContext() noexcept { return threadContext_; }
			private:
				ThreadContext_Arch threadContext_;
				uint32_t priority_;
				std::unique_ptr<uint8_t[]> stack_;
			};

			class Process
			{
			public:
				Process(std::string_view name);

				HANDLE AddThread(ThreadMain_t entryPoint, uint32_t priority, uintptr_t parameter);
			private:
				std::string name_;
				Chino::list<Process> processes_;
				Chino::list<Thread> threads_;
			};

			typedef Chino::list<HANDLE>::iterator thread_handle_it;
			typedef Chino::list<Thread>::iterator thread_it;
		public:
			ProcessManager();

			HANDLE CreateProcess(std::string_view name, uint32_t mainThreadPriority, ThreadMain_t entryPoint);
			void StartScheduler();
			ThreadContext_Arch& SwitchThreadContext();
		private:
			Process & GetProcess(HANDLE handle);
			void AddReadyThread(HANDLE handle);
			thread_handle_it SelectNextSwitchToThread();
		private:
			Chino::list<Process> _processes;
			std::array<Chino::list<HANDLE>, MAX_THREAD_PRIORITY + 1> readyThreads_;
			thread_handle_it runningThread_;
			HANDLE idleProcess_;
		};
	}
}

extern StaticHolder<Chino::Thread::ProcessManager> g_ProcessMgr;