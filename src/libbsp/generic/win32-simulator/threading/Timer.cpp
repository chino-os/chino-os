//
// Chino Thread
//
#include <Windows.h>
#include <libarch/arch.h>
#include <libbsp/bsp.hpp>
#include <mutex>
#include <process.h>

using namespace Chino::Threading;

#define portINITIAL_RFLAGS 0x206u

#define configTICK_RATE_HZ 10

static std::atomic<bool> _switchQueued = false, _allowSwitch = false, _switchPending = false;
static HANDLE _timerThread, _timer, _wfiEvent;

static void ArchQueueContextSwitch();

void Chino::Threading::BSPSetupSchedulerTimer()
{
    DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &_timerThread, 0, FALSE, DUPLICATE_SAME_ACCESS);
    SetThreadDescription(GetCurrentThread(), L"Scheduler");
    _wfiEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    _timer = CreateWaitableTimer(nullptr, FALSE, nullptr);
    ArchEnableInterrupt();
    while (true)
        SleepEx(INFINITE, TRUE);
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

static void ArchSwitchContext(ULONG_PTR)
{
    auto ctx = reinterpret_cast<ThreadContext_Arch*>(g_CurrentThreadContext);
    if (ctx)
    {
        SuspendThread((HANDLE)ctx->thread);

        CONTEXT hostCtx = { 0 };
        hostCtx.ContextFlags = CONTEXT_CONTROL;
        GetThreadContext((HANDLE)ctx->thread, &hostCtx);
    }

    Kernel_SwitchThreadContext();
    ctx = reinterpret_cast<ThreadContext_Arch*>(g_CurrentThreadContext);
    ResumeThread((HANDLE)ctx->thread);
    _switchQueued.store(false, std::memory_order_release);
}

static void ArchQueueContextSwitch()
{
    if (_allowSwitch)
    {
        bool exp = false;
        if (_switchQueued.compare_exchange_strong(exp, true, std::memory_order_acq_rel))
            QueueUserAPC(ArchSwitchContext, _timerThread, 0);
    }
    else
    {
        _switchPending = true;
    }
}

static void SchedulerTimerCallback(LPVOID lpArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
    if (Kernel_IncrementTick())
        ArchQueueContextSwitch();
}

static unsigned int ThreadStartThunk(void* args)
{
    auto context = reinterpret_cast<ThreadContext_Arch*>(args);
    auto threadMain = reinterpret_cast<void (*)(uintptr_t)>(context->entryPoint);
    threadMain(context->parameter);
    auto returnAddr = reinterpret_cast<void (*)()>(context->returnAddress);
    returnAddr();
    return 0;
}

extern "C"
{
    void ArchInitializeThreadContextArch(ThreadContext_Arch* context, uintptr_t stackPointer, uintptr_t entryPoint, uintptr_t returnAddress, uintptr_t parameter)
    {
        context->entryPoint = entryPoint;
        context->parameter = parameter;
        context->returnAddress = returnAddress;

        context->thread = _beginthreadex(nullptr, 0, ThreadStartThunk, context, CREATE_SUSPENDED, nullptr);
    }

    bool ArchValidateThreadContext(ThreadContext_Arch* context, uintptr_t stackTop, uintptr_t stackBottom)
    {
        return true;
    }

    void ArchDisableInterrupt()
    {
        _allowSwitch = false;
        CancelWaitableTimer(_timer);
    }

    void ArchEnableInterrupt()
    {
        LARGE_INTEGER dueTime;
        dueTime.QuadPart = 0;
        SetWaitableTimer(_timer, &dueTime, 1000 / configTICK_RATE_HZ, SchedulerTimerCallback, nullptr, FALSE);
        _allowSwitch = true;
        if (_switchPending)
        {
            _switchPending = false;
            ArchQueueContextSwitch();
        }
    }

    void ArchHaltProcessor()
    {
        WaitForSingleObject(_wfiEvent, INFINITE);
    }
}
