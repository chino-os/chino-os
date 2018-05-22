//
// Chino Arch Arch
//
#include "../arch.h"
#include <climits>
#include <kernel/kdebug.hpp>

/* Constants required to manipulate the core.  Registers first... */
#define portNVIC_SYSTICK_CTRL_REG			( * ( ( volatile uint32_t * ) 0xe000e010 ) )
#define portNVIC_SYSTICK_LOAD_REG			( * ( ( volatile uint32_t * ) 0xe000e014 ) )
#define portNVIC_SYSTICK_CURRENT_VALUE_REG	( * ( ( volatile uint32_t * ) 0xe000e018 ) )
#define portNVIC_SYSPRI2_REG				( * ( ( volatile uint32_t * ) 0xe000ed20 ) )
/* ...then bits in the registers. */
#define portNVIC_SYSTICK_INT_BIT			( 1UL << 1UL )
#define portNVIC_SYSTICK_ENABLE_BIT			( 1UL << 0UL )
#define portNVIC_SYSTICK_COUNT_FLAG_BIT		( 1UL << 16UL )
#define portNVIC_PENDSVCLEAR_BIT 			( 1UL << 27UL )
#define portNVIC_PEND_SYSTICK_CLEAR_BIT		( 1UL << 25UL )

#ifndef configKERNEL_INTERRUPT_PRIORITY
#define configKERNEL_INTERRUPT_PRIORITY 255
#endif

#define portNVIC_PENDSV_PRI					( ( ( uint32_t ) configKERNEL_INTERRUPT_PRIORITY ) << 16UL )
#define portNVIC_SYSTICK_PRI				( ( ( uint32_t ) configKERNEL_INTERRUPT_PRIORITY ) << 24UL )

#define configCPU_CLOCK_HZ 12000000
#define configTICK_RATE_HZ 10

#define configSYSTICK_CLOCK_HZ configCPU_CLOCK_HZ
/* Ensure the SysTick is clocked at the same frequency as the core. */
#define portNVIC_SYSTICK_CLK_BIT	( 1UL << 2UL )

#define portNVIC_INT_CTRL_REG		( * ( ( volatile uint32_t * ) 0xe000ed04 ) )
#define portNVIC_PENDSVSET_BIT		( 1UL << 28UL )

/* Constants required to set up the initial stack. */
#define portINITIAL_XPSR					( 0x01000000UL )

extern "C"
{
	void SysTick_Handler();

	void ArchInitializeThreadContextArch(ThreadContext_Arch* context, uintptr_t stackPointer, uintptr_t entryPoint, uintptr_t returnAddress, uintptr_t parameter)
	{
		context->r[0] = parameter;
		context->sp = stackPointer - sizeof(uint32_t) * 8;

		context->lr = returnAddress;
		context->pc = entryPoint;
		context->psr = portINITIAL_XPSR;
	}

	void ArchSetupSchedulerTimer()
	{
		/* Make PendSV and SysTick the lowest priority interrupts. */
		portNVIC_SYSPRI2_REG |= portNVIC_PENDSV_PRI;
		portNVIC_SYSPRI2_REG |= portNVIC_SYSTICK_PRI;

		portNVIC_SYSTICK_CTRL_REG = 0UL;
		portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;

		/* Configure SysTick to interrupt at the requested rate. */
		portNVIC_SYSTICK_LOAD_REG = (configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ) - 1UL;
		portNVIC_SYSTICK_CTRL_REG = (portNVIC_SYSTICK_CLK_BIT | portNVIC_SYSTICK_INT_BIT | portNVIC_SYSTICK_ENABLE_BIT);
	}

	void ArchSaveThreadContextArch(ThreadContext_Arch* tcontext, InterruptContext_Arch* icontext)
	{
		memcpy(&tcontext->r, icontext->r, sizeof(icontext->r));
		tcontext->sp = icontext->sp_before;
		tcontext->lr = icontext->lr_before;
		tcontext->pc = icontext->pc_before;
		tcontext->psr = icontext->psr_before;
	}

	void ArchSleepMs(uint32_t ms)
	{
		auto count = configCPU_CLOCK_HZ / 1000 * ms;
		volatile int a;
		for (size_t i = 0; i < count; i++) a++;
	}

	void SysTick_Handler()
	{
		static uint32_t i = 0;
		if (i++ % 100 == 0)
		{
			/* The SysTick runs at the lowest interrupt priority, so when this interrupt
			executes all interrupts must be unmasked.  There is therefore no need to
			save and then restore the interrupt mask value as its value is already
			known. */
			ArchDisableInterrupt();
			{
				portNVIC_INT_CTRL_REG |= portNVIC_PENDSVSET_BIT;
			}
			ArchEnableInterrupt();
		}
	}
}
