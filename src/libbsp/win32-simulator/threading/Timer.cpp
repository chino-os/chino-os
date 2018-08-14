//
// Chino Thread
//
#include <libbsp/bsp.hpp>
#include <libarch/arch.h>
#include <Windows.h>
#include <process.h>
#include <mutex>

using namespace Chino::Threading;

#define portINITIAL_RFLAGS	0x206u

#define configTICK_RATE_HZ 10

static std::atomic<bool> _switchQueued = false;
static HANDLE _timerThread, _timer, _workerThread, _wfiEvent;

static void ArchQueueContextSwitch();

static void TimerThreadMain(void*)
{
	_timer = CreateWaitableTimer(nullptr, FALSE, nullptr);
	ArchEnableInterrupt();
	while (true)
		SleepEx(INFINITE, TRUE);
}

void Chino::Threading::BSPSetupSchedulerTimer()
{
	_wfiEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &_workerThread, 0, FALSE, DUPLICATE_SAME_ACCESS);
	_timerThread = (HANDLE)_beginthread(TimerThreadMain, 1024, nullptr);

	while (1);
}

void Chino::Threading::BSPSleepMs(uint32_t ms)
{
	Sleep(ms);
}

void Chino::Threading::BSPYield()
{
	ArchQueueContextSwitch();
}

size_t Chino::Threading::BSPMsToTicks(size_t ms)
{
	return configTICK_RATE_HZ * ms / 1000;
}

struct InterruptService
{
	InterruptService()
	{
		SuspendThread(_workerThread);
		SetEvent(_wfiEvent);
	}

	~InterruptService()
	{
		ResumeThread(_workerThread);
	}
};

static std::atomic<bool> _exitKernel;

static void WaitForExitKernelMode()
{
	_exitKernel.store(true, std::memory_order_release);
	while (1);
}

static std::mutex mutex;

static void ArchSwitchContext(ULONG_PTR)
{
	std::lock_guard<std::mutex> locker(mutex);
	CONTEXT hostCtx = { 0 };
	hostCtx.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;

	{
		_exitKernel.store(false, std::memory_order_release);
		SuspendThread(_workerThread);
		GetThreadContext(_workerThread, &hostCtx);
		auto newCtx = hostCtx;
		newCtx.Rip = DWORD64(WaitForExitKernelMode);
		SetThreadContext(_workerThread, &newCtx);
		ResumeThread(_workerThread);
		while (!_exitKernel.load(std::memory_order_acquire));
	}

	{
		InterruptService is;
		auto ctx = reinterpret_cast<ThreadContext_Arch*>(g_CurrentThreadContext);

		// Save
		if (ctx)
		{
			ctx->rax = hostCtx.Rax;
			ctx->rbx = hostCtx.Rbx;
			ctx->rcx = hostCtx.Rcx;
			ctx->rdx = hostCtx.Rdx;
			ctx->rbp = hostCtx.Rbp;
			ctx->rdi = hostCtx.Rdi;
			ctx->rsi = hostCtx.Rsi;
			ctx->r8 = hostCtx.R8;
			ctx->r9 = hostCtx.R9;
			ctx->r10 = hostCtx.R10;
			ctx->r11 = hostCtx.R11;
			ctx->r12 = hostCtx.R12;
			ctx->r13 = hostCtx.R13;
			ctx->r14 = hostCtx.R14;
			ctx->r15 = hostCtx.R15;

			ctx->rip = hostCtx.Rip;
			ctx->eflags = hostCtx.EFlags;
			ctx->rsp = hostCtx.Rsp;
		}

		// Restore
		Kernel_SwitchThreadContext();
		ctx = reinterpret_cast<ThreadContext_Arch*>(g_CurrentThreadContext);
		hostCtx.Rax = ctx->rax;
		hostCtx.Rbx = ctx->rbx;
		hostCtx.Rcx = ctx->rcx;
		hostCtx.Rdx = ctx->rdx;
		hostCtx.Rbp = ctx->rbp;
		hostCtx.Rdi = ctx->rdi;
		hostCtx.Rsi = ctx->rsi;
		hostCtx.R8 = ctx->r8;
		hostCtx.R9 = ctx->r9;
		hostCtx.R10 = ctx->r10;
		hostCtx.R11 = ctx->r11;
		hostCtx.R12 = ctx->r12;
		hostCtx.R13 = ctx->r13;
		hostCtx.R14 = ctx->r14;
		hostCtx.R15 = ctx->r15;

		hostCtx.Rip = ctx->rip;
		hostCtx.EFlags = ctx->eflags;
		hostCtx.Rsp = ctx->rsp;

		SetThreadContext(_workerThread, &hostCtx);
		_switchQueued.store(false, std::memory_order_release);
	}
}

static void ArchQueueContextSwitch()
{
	bool exp = false;
	if (_switchQueued.compare_exchange_strong(exp, true, std::memory_order_acq_rel))
		QueueUserAPC(ArchSwitchContext, _timerThread, 0);
}

static void SchedulerTimerCallback(LPVOID lpArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
	InterruptService is;
	if (Kernel_IncrementTick())
		ArchQueueContextSwitch();
}

extern "C"
{
	void ArchInitializeThreadContextArch(ThreadContext_Arch* context, uintptr_t stackPointer, uintptr_t entryPoint, uintptr_t returnAddress, uintptr_t parameter)
	{
		auto stack = reinterpret_cast<uint64_t*>(stackPointer);

		context->rcx = parameter;
		context->eflags = portINITIAL_RFLAGS;
		context->rip = entryPoint;

		--stack;
		*--stack = returnAddress;
		context->rsp = uintptr_t(stack);
	}

	void ArchDisableInterrupt()
	{
		CancelWaitableTimer(_timer);
	}

	void ArchEnableInterrupt()
	{
		LARGE_INTEGER dueTime;
		dueTime.QuadPart = 0;
		SetWaitableTimer(_timer, &dueTime, 1000 / configTICK_RATE_HZ, SchedulerTimerCallback, nullptr, FALSE);
	}

	void ArchHaltProcessor()
	{
		while (WaitForSingleObject(_wfiEvent, 10) != WAIT_OBJECT_0)
			Sleep(0);
	}
}
