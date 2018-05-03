//
// Chino Thread
//
#include "ProcessManager.hpp"
#include "../kdebug.hpp"

using namespace Chino::Thread;

static void OnThreadExit();
static void IdleThreadMain(uintptr_t);

template<typename T>
typename Chino::list<T>::iterator HandleToListIt(HANDLE handle)
{
	using iterator = typename Chino::list<T>::iterator;
	return iterator(reinterpret_cast<typename Chino::list<T>::node*>(handle));
}

template<typename T>
HANDLE ToHandle(typename Chino::list<T>::iterator value)
{
	return reinterpret_cast<HANDLE>(value.node_);
}

ProcessManager::ProcessManager()
	:runningThread_(0), idleProcess_(0)
{

}

HANDLE ProcessManager::CreateProcess(std::string_view name, uint32_t mainThreadPriority, ThreadMain_t entryPoint)
{
	auto it = _processes.emplace_back(name);
	it->AddThread(entryPoint, mainThreadPriority, 0);
	return ToHandle<Process>(it);
}

void ProcessManager::AddReadyThread(HANDLE handle)
{
	auto it = HandleToListIt<Thread>(handle);
	auto priority = it->GetPriority();
	kassert(priority < readyThreads_.size());
	readyThreads_[priority].emplace_back(handle);
	kassert(!readyThreads_[priority].empty());
}

void ProcessManager::StartScheduler()
{
	kassert(!idleProcess_);
	idleProcess_ = CreateProcess("System Idle", 0, IdleThreadMain);

	PortSetupSchedulerTimer();
	while (1);
}

ProcessManager::thread_handle_it ProcessManager::SelectNextSwitchToThread()
{
	// Round robin in threads of same priority
	thread_handle_it threadSwitchTo;
	if (runningThread_.good())
	{
		auto next = runningThread_;
		if ((++next).good())
			threadSwitchTo = next;

		g_BootVideo->PutFormat(L"Now (%lx), Next (%lx)\n", runningThread_.node_, next.node_);
	}
	
	if(!threadSwitchTo.good())
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
	g_BootVideo->PutFormat(L"SwitchThreadContext End: %lx\n", threadSwitchTo.node_);
	return threadSwitchTo;
}

void ProcessManager::SwitchThreadContext(InterruptContext& context)
{
	g_BootVideo->PutString(L"SwitchThreadContext\n");
	auto nextThread = SelectNextSwitchToThread();
	if (runningThread_ != nextThread)
	{
		auto oldThread = runningThread_.good() ? HandleToListIt<Thread>(*runningThread_) : thread_it();
		auto newThread = HandleToListIt<Thread>(*nextThread);

		g_BootVideo->PutFormat(L"Running (%lx), New (%lx)\n", oldThread.node_, newThread.node_);

		if (oldThread.good())
			oldThread->SwitchOut(context);

		runningThread_ = nextThread;
		newThread->SwitchIn(context);
	}
	g_BootVideo->PutString(L"SwitchThreadContext No\n");
}

ProcessManager::Process & ProcessManager::GetProcess(HANDLE handle)
{
	kassert(handle);
	return *HandleToListIt<Process>(handle);
}

ProcessManager::Process::Process(std::string_view name)
	:name_(name)
{
}

HANDLE ProcessManager::Process::AddThread(ThreadMain_t entryPoint, uint32_t priority, uintptr_t parameter)
{
	auto handle = ToHandle<Thread>(threads_.emplace_back(entryPoint, priority, parameter));
	g_ProcessMgr->AddReadyThread(handle);
	return handle;
}

ProcessManager::Thread::Thread(ThreadMain_t entryPoint, uint32_t priority, uintptr_t parameter)
	:priority_(priority), threadContext_({})
{
	kassert(priority <= MAX_THREAD_PRIORITY);
	stack_ = std::make_unique<uint8_t[]>(DEFAULT_THREAD_STACK_SIZE);
	PortInitializeThreadContextArch(&threadContext_.arch, uintptr_t(stack_.get()), uintptr_t(entryPoint), uintptr_t(OnThreadExit), parameter);
}

void ProcessManager::Thread::SwitchOut(InterruptContext& context)
{
	g_BootVideo->PutFormat(L"Switch out (%lx)\n", this);
	PortSaveThreadContextArch(&threadContext_.arch, &context.arch);
}

void ProcessManager::Thread::SwitchIn(InterruptContext& context)
{
	g_BootVideo->PutFormat(L"Switch in (%lx)\n", this);
	g_BootVideo->PutFormat(L"B RFLAGS: %lx, RIP: %lx, RSP: %lx\n", context.arch.rflags, context.arch.rip_before, context.arch.rsp_before);
	PortRestoreThreadContextArch(&threadContext_.arch, &context.arch);
}

static void OnThreadExit()
{
	kassert(!"Exit unexpected.");
}

static void IdleThreadMain(uintptr_t)
{
	while (1)
	{
		PortHaltProcessor();

		for (size_t i = 0; i < 1000; i++);
		g_BootVideo->PutChar(L'.');
	}
}

extern "C" void Kernel_OnTimerHandler(void* interruptContext)
{
	static uint64_t count = 0;
	Chino::InterruptContext context;
	context.arch = *reinterpret_cast<InterruptContext_Arch*>(interruptContext);

	if (count++ % 200 == 0)
	{
		g_BootVideo->PutFormat(L"\nA RFLAGS: %lx, RIP: %lx, RSP: %lx\n", context.arch.rflags, context.arch.rip_before, context.arch.rsp_before);
		g_ProcessMgr->SwitchThreadContext(context);
	}
	//while (1);
}
