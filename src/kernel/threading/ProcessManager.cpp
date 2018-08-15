//
// Chino Threading
//
#include "ProcessManager.hpp"
#include <libbsp/bsp.hpp>
#include "../kdebug.hpp"

extern "C"
{
	uintptr_t g_CurrentThreadContext = 0;
}

using namespace Chino;
using namespace Chino::Threading;

std::atomic<size_t> kernel_critical::coreTaken_(0);
std::atomic<size_t> kernel_critical::depth_(0);

#define IdleThreadStackSize 512

static void OnThreadExit();
static void IdleThreadMain();

ProcessManager::ProcessManager()
	:runningThread_(0), idleProcess_(nullptr), tickCount_(0)
{

}

ObjectPtr<Process> ProcessManager::CreateProcess(std::string_view name, uint32_t mainThreadPriority, std::function<void()> threadMain, size_t mainThreadStackSize)
{
	auto it = _processes.emplace_back(MakeObject<Process>(name));
	it->AddThread(std::move(threadMain), mainThreadPriority, mainThreadStackSize);
	return it;
}

void ProcessManager::AddReadyThread(ObjectPtr<Thread> thread)
{
	if (thread->state_ != ThreadState::Ready)
	{
		auto priority = thread->GetPriority();
		kassert(priority < readyThreads_.size());
		readyThreads_[priority].emplace_back(thread);
		thread->state_ = ThreadState::Ready;
		kassert(!readyThreads_[priority].empty());
	}
}

void ProcessManager::StartScheduler()
{
	kassert(!idleProcess_);
	idleProcess_ = CreateProcess("System Idle", 0, IdleThreadMain, IdleThreadStackSize);

	BSPSetupSchedulerTimer();
	ArchHaltProcessor();
}

thread_it ProcessManager::SelectNextSwitchToThread()
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
	(*threadSwitchTo)->ValidateContext();
	return threadSwitchTo;
}

bool ProcessManager::IncrementTick()
{
	tickCount_++;
	while (!delayedThreads_.empty())
	{
		auto& top = delayedThreads_.top();
		if (tickCount_ >= top.Tick)
		{
			auto thread = top.Thread;
			if (top.DelayToken == (*thread)->delayToken_)
			{
				(*thread)->weakupReason_ = ThreadWakeupReason::Timeout;
				AttachReadyThread(thread);
			}

			delayedThreads_.pop();
		}
		else
		{
			break;
		}
	}

	auto nextThread = SelectNextSwitchToThread();
	nextThread_ = nextThread;
	return nextThread != runningThread_;
}

void ProcessManager::SwitchThreadContext()
{
	auto nextThread = nextThread_;
	runningThread_ = nextThread;
	g_CurrentThreadContext = uintptr_t(&(*nextThread)->GetContext());
}

ObjectPtr<Thread> ProcessManager::GetCurrentThread()
{
	assert(runningThread_.good());
	return *runningThread_;
}

thread_it ProcessManager::DetachCurrentThread()
{
	auto thread = runningThread_;
	assert(thread.good());
	auto priority = (*thread)->GetPriority();
	readyThreads_[priority].detach(thread);
	(*thread)->state_ = ThreadState::Blocked;
	(*thread)->weakupReason_ = ThreadWakeupReason::None;
	nextThread_ = SelectNextSwitchToThread();
	BSPYield();
	return thread;
}

void ProcessManager::AttachReadyThread(thread_it thread)
{
	assert(thread.good());
	if ((*thread)->state_ != ThreadState::Ready)
	{
		auto priority = (*thread)->GetPriority();
		readyThreads_[priority].attach(thread);
		(*thread)->state_ = ThreadState::Ready;

		if (!runningThread_.good() || priority >= (*runningThread_)->GetPriority())
		{
			auto nextThread = SelectNextSwitchToThread();
			nextThread_ = nextThread;
			if (nextThread != runningThread_)
				BSPYield();
		}
	}
}

void ProcessManager::DelayThread(thread_it thread, std::chrono::milliseconds timeout)
{
	assert(thread.good());
	auto ticks = BSPMsToTicks(timeout.count());
	delayedThreads_.emplace(tickCount_ + ticks, thread, ++(*thread)->delayToken_);
}

void ProcessManager::SleepCurrentThread(std::chrono::milliseconds timeout)
{
	kernel_critical kc;
	DelayThread(DetachCurrentThread(), timeout);
}

Process::Process(std::string_view name)
	:name_(name)
{
}

ObjectPtr<Thread> Process::AddThread(std::function<void()> threadMain, uint32_t priority, size_t stackSize)
{
	auto thread = threads_.emplace_back(MakeObject<Thread>(this, std::move(threadMain), priority, stackSize));
	g_ProcessMgr->AddReadyThread(thread);
	return thread;
}

Thread::Thread(ObjectPtr<Process> process, std::function<void()> threadMain, uint32_t priority, size_t stackSize)
	:process_(process), priority_(priority), threadContext_({}), threadMain_(std::move(threadMain)), state_(ThreadState::Invalid),
	stackSize_(stackSize), weakupReason_(ThreadWakeupReason::None), delayToken_(0)
{
	kassert(threadMain_ && priority <= MAX_THREAD_PRIORITY);
	kassert(stackSize && stackSize % sizeof(uintptr_t) == 0);
	stack_ = std::make_unique<uint8_t[]>(stackSize);
	auto stackPointer = uintptr_t(stack_.get()) + stackSize;
	ArchInitializeThreadContextArch(&threadContext_, stackPointer, uintptr_t(ThreadMainThunk), uintptr_t(OnThreadExit), uintptr_t(this));
}

void Thread::ThreadMainThunk(Thread* thread)
{
	kassert(thread && thread->threadMain_);
	thread->threadMain_();
}

void Thread::ValidateContext()
{
	if (!ArchValidateThreadContext(&GetContext(), uintptr_t(stack_.get()) + stackSize_, uintptr_t(stack_.get())))
		kassert(!"Stack overflow.");
}

static void OnThreadExit()
{
	kassert(!"Exit unexpected.");
}

static void IdleThreadMain()
{
	while (1)
	{
		for (size_t i = 0; i < 100; i++)
			ArchHaltProcessor();
		g_Logger->PutChar(L'.');
	}
}

kernel_critical::kernel_critical()
{
	size_t coreId = 1;
	if (coreTaken_.load(std::memory_order_acquire) != coreId)
	{
		size_t expected = 0;
		while (!coreTaken_.compare_exchange_strong(expected, coreId, std::memory_order_acq_rel))
			expected = 0;

		ArchDisableInterrupt();
	}

	depth_++;
}

kernel_critical::~kernel_critical()
{
	if (--depth_ == 0)
	{
		coreTaken_.store(0, std::memory_order_release);
		ArchEnableInterrupt();
	}
}

extern "C" bool Kernel_IncrementTick()
{
	return g_ProcessMgr->IncrementTick();
}

extern "C" void Kernel_SwitchThreadContext()
{
	g_ProcessMgr->SwitchThreadContext();
}
