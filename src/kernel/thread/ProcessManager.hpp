//
// Chino Thread
//
#pragma once
#include "../../libchino/chino.h"
#include <portable.h>
#include <list.hpp>
#include <string_view>
#include <string>

namespace Chino
{
	namespace Thread
	{
		class ProcessManager
		{
			struct ThreadContext
			{
				ThreadContext_Arch arch_;
			};

			class Thread
			{
			public:
				Thread(uintptr_t entryPoint);
			private:
				ThreadContext threadContext_;
			};

			class Process
			{
			public:
				Process(std::string_view name);

				HANDLE AddThread(uintptr_t entryPoint);
			private:
				std::string name_;
				Chino::list<Process> processes_;
				Chino::list<Thread> threads_;
			};
		public:
			ProcessManager();

			HANDLE CreateProcess(std::string_view name, uintptr_t entryPoint);
		private:
			Process & GetProcess(HANDLE handle);
		private:
			Chino::list<Process> processes_;
		};
	}
}