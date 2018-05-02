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
}

void ProcessManager::StartScheduler()
{
	kassert(!idleProcess_);
	idleProcess_ = CreateProcess("System Idle", 0, IdleThreadMain);
	PortSetupSchedulerTimer();

	__asm volatile("sti");
	while (1);
}

ProcessManager::Process & ProcessManager::GetProcess(HANDLE handle)
{
	kassert(handle);
	return **HandleToListIt<Process>(handle);
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

static void OnThreadExit()
{
	kassert(!"Exit unexpected.");
}

static void IdleThreadMain(uintptr_t)
{
	while (1)
	{
		PortHaltProcessor();
	}
}

extern "C" void vPortTimerHandler()
{
	kassert(!"haha");
}
