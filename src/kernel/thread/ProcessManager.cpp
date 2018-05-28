//
// Chino Thread
//
#include "ProcessManager.hpp"
#include "../kdebug.hpp"

extern "C"
{
	uintptr_t g_CurrentThreadContext = 0;
}

using namespace Chino::Thread;

static void OnThreadExit();
static void IdleThreadMain(uintptr_t);

ProcessManager::ProcessManager()
	:runningThread_(0), idleProcess_(nullptr)
{

}

Process& ProcessManager::CreateProcess(std::string_view name, uint32_t mainThreadPriority, ThreadMain_t entryPoint)
{
	auto it = _processes.emplace_back(MakeObject<Process>(name));
	it->AddThread(entryPoint, mainThreadPriority, 0);
	return *it;
}

void ProcessManager::AddReadyThread(Thread& thread)
{
	auto priority = thread.GetPriority();
	kassert(priority < readyThreads_.size());
	readyThreads_[priority].emplace_back(&thread);
	kassert(!readyThreads_[priority].empty());
}

void ProcessManager::StartScheduler()
{
	kassert(!idleProcess_);
	idleProcess_.Reset(&CreateProcess("System Idle", 0, IdleThreadMain));

	ArchSetupSchedulerTimer();
	ArchHaltProcessor();
}

ProcessManager::thread_it ProcessManager::SelectNextSwitchToThread()
{
	// Round robin in threads of same priority
	thread_it threadSwitchTo;
	if (runningThread_.good())
	{
		auto next = runningThread_;
		if ((++next).good())
			threadSwitchTo = next;
	}

	if (!threadSwitchTo.good())
	{
		for (auto it = readyThreads_.rbegin(); it != readyThreads_.rend(); ++it)
		{
			if (!it->empty())
			{
				threadSwitchTo = it->begin();
				break;
			}
		}
	}

	kassert(threadSwitchTo.good());
	return threadSwitchTo;
}

ThreadContext_Arch& ProcessManager::SwitchThreadContext()
{
	auto nextThread = SelectNextSwitchToThread();
	runningThread_ = nextThread;
	auto& arch = (*nextThread)->GetContext();
#if 0
	g_Logger->PutFormat("PSP: %x\n", arch.sp);
#endif
	return arch;
}

Process::Process(std::string_view name)
	:name_(name)
{
}

Thread& Process::AddThread(ThreadMain_t entryPoint, uint32_t priority, uintptr_t parameter)
{
	auto& thread = *threads_.emplace_back(MakeObject<Thread>(entryPoint, priority, parameter));
	g_ProcessMgr->AddReadyThread(thread);
	return thread;
}

Thread::Thread(ThreadMain_t entryPoint, uint32_t priority, uintptr_t parameter)
	:priority_(priority), threadContext_({})
{
	kassert(priority <= MAX_THREAD_PRIORITY);
	auto stackSize = DEFAULT_THREAD_STACK_SIZE;
	stack_ = std::make_unique<uint8_t[]>(stackSize);
	auto stackPointer = uintptr_t(stack_.get()) + stackSize;
	ArchInitializeThreadContextArch(&threadContext_, stackPointer, uintptr_t(entryPoint), uintptr_t(OnThreadExit), parameter);
}

static void OnThreadExit()
{
	kassert(!"Exit unexpected.");
}

static void IdleThreadMain(uintptr_t)
{
	while (1)
	{
		for (size_t i = 0; i < 100; i++)
			ArchHaltProcessor();
		g_Logger->PutChar(L'.');
	}
}

extern "C" void Kernel_SwitchThreadContext()
{
	g_CurrentThreadContext = uintptr_t(&g_ProcessMgr->SwitchThreadContext());
}
