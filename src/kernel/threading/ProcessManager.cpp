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
	auto priority = thread->GetPriority();
	kassert(priority < readyThreads_.size());
	readyThreads_[priority].emplace_back(thread);
	kassert(!readyThreads_[priority].empty());
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
	return threadSwitchTo;
}

bool ProcessManager::IncrementTick()
{
	tickCount_++;
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
	nextThread_ = SelectNextSwitchToThread();
	BSPYield();
	return thread;
}

void ProcessManager::AttachReadyThread(thread_it thread)
{
	assert(thread.good());
	auto priority = (*thread)->GetPriority();
	readyThreads_[priority].attach(thread);

	if (!runningThread_.good() || priority >= (*runningThread_)->GetPriority())
	{
		auto nextThread = SelectNextSwitchToThread();
		nextThread_ = nextThread;
		if (nextThread != runningThread_)
			BSPYield();
	}
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
	:process_(process), priority_(priority), threadContext_({}), threadMain_(std::move(threadMain))
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

#include "timer.h"
uint16_t current_clock = 0;
/*
********************************************************************************
* 函 数 名: clock_time
* 功能说明: 返回当前计数值
* 参    数：无
* 返 回 值: uint16_t current_clock  当前计数时间
* 使用说明：
* 调用方法：clock_time()
********************************************************************************
*/
uint16_t clock_time(void)
{
	return current_clock;
}

/*
********************************************************************************
* 函 数 名: timer_set
* 功能说明: 设定定时器定时间隔时间
* 参    数：timer_typedef* ptimer   定时器结构体
*           uint16_t interval       间隔时间
* 返 回 值: 无
* 使用说明：
* 调用方法：timer_set(&arp_timer,10000);
********************************************************************************
*/
void timer_set(timer_typedef* ptimer, uint16_t interval)
{
	/* 设置时间间隔 */
	ptimer->interval = interval;
	/* 设置启动时间 */
	ptimer->start = clock_time();
}

/*
********************************************************************************
* 函 数 名: timer_reset
* 功能说明: 重新设定定时器
* 参    数：timer_typedef* ptimer   定时器结构体
* 返 回 值: 无
* 使用说明：
* 调用方法：timer_reset(&arp_timer);
********************************************************************************
*/
void timer_reset(timer_typedef * ptimer)
{
	ptimer->start = ptimer->start + ptimer->interval;
}

/*
********************************************************************************
* 函 数 名: timer_expired
* 功能说明: 查询定时器是否溢出
* 参    数：timer_typedef* ptimer   定时器结构体
* 返 回 值: int8_t                  定时器是否溢出 1 代表溢出 0代表未溢出
* 使用说明：
* 调用方法：timer_expired(&arp_timer)
********************************************************************************
*/
int8_t timer_expired(timer_typedef* ptimer)
{
	/* 一定要装换为有符号数，进行数学比较时，多使用有符号数 */
	if ((int16_t)(clock_time() - ptimer->start) >= (int16_t)ptimer->interval)
		return 1;
	else
		return 0;
}


extern "C" bool Kernel_IncrementTick()
{
	/* 时间标志累加 */
	current_clock++;
	return g_ProcessMgr->IncrementTick();
}

extern "C" void Kernel_SwitchThreadContext()
{
	g_ProcessMgr->SwitchThreadContext();
}
