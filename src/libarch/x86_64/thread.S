#define REGBYTES 8

.global ArchHaltProcessor
.global ArchEnableInterrupt
.global ArchDisableInterrupt
.global ArchRestoreThreadContextArch
.global ArchTimerHandler
.global ArchAPICSpuriousHandler

ArchHaltProcessor:
	hlt

ArchEnableInterrupt:
	sti
	ret

ArchDisableInterrupt:
	cli
	ret
	
.align 8
ArchAPICSpuriousHandler:
	iretq
	
.align 8
ArchTimerHandler:
	pushq %rax
	mov g_CurrentThreadContext@GOTPCREL(%rip), %rax
	mov (%rax), %rax
	/* if g_CurrentThreadContext == 0, skip save */
	test %rax, %rax
	jz fvck

	mov %rbx, 1 * REGBYTES(%rax)
	mov %rcx, 2 * REGBYTES(%rax)
	mov %rdx, 3 * REGBYTES(%rax)
	mov %rbp, 4 * REGBYTES(%rax)
	mov %rdi, 5 * REGBYTES(%rax)
	mov %rsi, 6 * REGBYTES(%rax)
	mov %r8,  7 * REGBYTES(%rax)
	mov %r9,  8 * REGBYTES(%rax)
	mov %r10, 9 * REGBYTES(%rax)
	mov %r11, 10 * REGBYTES(%rax)
	mov %r12, 11 * REGBYTES(%rax)
	mov %r13, 12 * REGBYTES(%rax)
	mov %r14, 13 * REGBYTES(%rax)
	mov %r15, 14 * REGBYTES(%rax)

	mov 1 * REGBYTES(%rsp),	%rbx
	mov %rbx, 15 * REGBYTES(%rax) # rip
	mov 2 * REGBYTES(%rsp),	%rbx
	mov %rbx, 16 * REGBYTES(%rax) # cs
	mov 3 * REGBYTES(%rsp),	%rbx
	mov %rbx, 17 * REGBYTES(%rax) # rflags
	mov 4 * REGBYTES(%rsp),	%rbx
	mov %rbx, 18 * REGBYTES(%rax) # rsp
	mov %rax, %rbx
	popq %rax
	mov %rax, 0 * REGBYTES(%rbx)
	
restore:
	call Kernel_SwitchThreadContext

	mov g_CurrentThreadContext@GOTPCREL(%rip), %rax
	mov (%rax), %rax
	mov 2 * REGBYTES(%rax), %rcx
	mov 3 * REGBYTES(%rax), %rdx
	mov 4 * REGBYTES(%rax), %rbp
	mov 5 * REGBYTES(%rax), %rdi
	mov 6 * REGBYTES(%rax), %rsi
	mov 7 * REGBYTES(%rax), %r8
	mov 8 * REGBYTES(%rax), %r9
	mov 9 * REGBYTES(%rax), %r10
	mov 10 * REGBYTES(%rax), %r11
	mov 11 * REGBYTES(%rax), %r12
	mov 12 * REGBYTES(%rax), %r13
	mov 13 * REGBYTES(%rax), %r14
	mov 14 * REGBYTES(%rax), %r15

	mov 15 * REGBYTES(%rax), %rbx
	mov %rbx, 0 * REGBYTES(%rsp) # rip
	mov 16 * REGBYTES(%rax), %rbx
	mov %rbx, 1 * REGBYTES(%rsp) # cs
	mov 17 * REGBYTES(%rax), %rbx
	mov %rbx, 2 * REGBYTES(%rsp) # rflags
	mov 18 * REGBYTES(%rax), %rbx
	mov %rbx, 3 * REGBYTES(%rsp) # rsp
	
	# EOI
	mov $0xFEE000B0, %rbx
	movl $0, (%rbx)

	mov 1 * REGBYTES(%rax), %rbx
	mov 0 * REGBYTES(%rax), %rax

	iretq
fvck:
	popq %rax
	jmp restore
