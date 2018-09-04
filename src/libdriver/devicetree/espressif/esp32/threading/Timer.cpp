//
// Chino Thread
//
#include <libbsp/bsp.hpp>
#include <libarch/arch.h>

using namespace Chino::Threading;

#define configCPU_CLOCK_HZ 72000000
#define configTICK_RATE_HZ 10

void Chino::Threading::BSPSetupSchedulerTimer()
{
}

void Chino::Threading::BSPSleepMs(uint32_t ms)
{
	auto count = configCPU_CLOCK_HZ / 1000 * ms / 3;
	for (size_t i = 0; i < count; i++)
		__asm volatile ("nop");
}

void Chino::Threading::BSPYield()
{
}

size_t Chino::Threading::BSPMsToTicks(size_t ms)
{
}

extern "C" void SysTick_Handler()
{
	/* The SysTick runs at the lowest interrupt priority, so when this interrupt
	executes all interrupts must be unmasked.  There is therefore no need to
	save and then restore the interrupt mask value as its value is already
	known. */
	ArchDisableInterrupt();

	if (Kernel_IncrementTick())
		BSPYield();

	ArchEnableInterrupt();
}
