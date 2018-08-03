#define REGBYTES 4
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
	cpsie i
	bx lr

ArchDisableInterrupt:
	cpsid i
	bx lr
	
  .type  PendSV_Handler, %function
PendSV_Handler:
	cpsid	i
	ldr		r0,		=g_CurrentThreadContext
	ldr		r0,		[r0]
	cbz		r0,		restore
	stmia	r0!,	{r4-r11}
	mrs		r1,		psp
	str		r1,		[r0]
restore:
	push	{lr}
	bl		Kernel_SwitchThreadContext
	pop		{lr}
	
	ldr		r0,		=g_CurrentThreadContext
	ldr		r0,		[r0]
	ldmia	r0!,	{r4-r11}
	ldr		r1,		[r0]
	msr		psp,	r1
	orr		lr, lr,	#0x04
	cpsie	i
	bx		lr
