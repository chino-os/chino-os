#define REGBYTES 4
#define InterruptStructSize 68
#define MAXBasePriority 0xE0

  .syntax unified
  .fpu softvfp
  .thumb

.global ArchHaltProcessor
.global ArchEnableInterrupt
.global ArchDisableInterrupt
.global ArchRestoreThreadContextArch
.global ArchTimerHandler
.global ArchAPICSpuriousHandler

ArchHaltProcessor:
	wfi
	bx lr

ArchEnableInterrupt:
	mov r0,	#0
	msr basepri, r0
	bx lr

ArchDisableInterrupt:
	mov r0,	#MAXBasePriority
	msr basepri, r0
	isb
	dsb
	bx lr

ArchRestoreThreadContextArch:
	bx lr
	
.align 4
PendSV_Handler:
	sub		sp,		#InterruptStructSize
	stmia	sp,		{r0-r12}
	mov		r0,		sp
	add		r0,		#InterruptStructSize
	str		r0,		[sp, #13 * REGBYTES]
	add		r0,		#REGBYTES
	ldmia	r0,		{r1,r2,r3}			@lr, psr, pc
	str		r1,		[sp, #14 * REGBYTES]
	str		r2,		[sp, #15 * REGBYTES]
	str		r3,		[sp, #16 * REGBYTES]
	
	mov		r0,		sp					@r0 = &icontext
	push	{lr}
	bl		Kernel_OnTimerHandler
	pop		{r0}
	add		sp,		#InterruptStructSize
	bx		r0
