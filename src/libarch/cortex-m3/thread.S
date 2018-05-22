#define REGBYTES 4
#define InterruptStructSize 76
#define MAXBasePriority 0xE0

  .syntax unified
  .fpu softvfp
  .thumb

.global ArchHaltProcessor
.global ArchEnableInterrupt
.global ArchDisableInterrupt
.global ArchRestoreThreadContextArch
.global PendSV_Handler

.section .text

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
	ldr		sp,		[r1, #17 * REGBYTES]
	ldr		lr,		[r1, #18 * REGBYTES]
	ldr		r2,		[r0, #13 * REGBYTES] @sp
	msr		psp,	r2
	ldr		r3,		[r0, #0 * REGBYTES]
	str		r3,		[r2, #0 * REGBYTES]
	ldr		r3,		[r0, #1 * REGBYTES]
	str		r3,		[r2, #1 * REGBYTES]
	ldr		r3,		[r0, #2 * REGBYTES]
	str		r3,		[r2, #2 * REGBYTES]
	ldr		r3,		[r0, #3 * REGBYTES]
	str		r3,		[r2, #3 * REGBYTES]
	ldr		r3,		[r0, #12 * REGBYTES]
	str		r3,		[r2, #4 * REGBYTES]
	ldr		r3,		[r0, #14 * REGBYTES]
	str		r3,		[r2, #5 * REGBYTES]
	ldr		r3,		[r0, #15 * REGBYTES]
	str		r3,		[r2, #6 * REGBYTES]
	ldr		r3,		[r0, #16 * REGBYTES]
	str		r3,		[r2, #7 * REGBYTES]
	orr		lr, lr,	#0x04
	cpsie	i
	bx lr
	
  .type  PendSV_Handler, %function
PendSV_Handler:
	cpsid	i
	sub		sp,		#InterruptStructSize
	stmia	sp,		{r0-r12}
	mov		r0,		sp
	add		r0,		#InterruptStructSize
	str		r0,		[sp, #17 * REGBYTES]
	mrs		r0,		psp
	str		r0,		[sp, #13 * REGBYTES]
	add		r0,		#REGBYTES * 5
	ldmia	r0,		{r1,r2,r3}			@lr, pc, psr
	str		r1,		[sp, #14 * REGBYTES]
	str		r2,		[sp, #15 * REGBYTES]
	str		r3,		[sp, #16 * REGBYTES]
	str		lr,		[sp, #18 * REGBYTES]
	
	mov		r0,		sp					@r0 = &icontext
	bl		Kernel_OnTimerHandler
	ldr		lr,		[sp, #18 * REGBYTES]
	add		sp,		#InterruptStructSize
	cpsie	i
	bx		lr
