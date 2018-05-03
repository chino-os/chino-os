#define REGBYTES 8
#define InterruptStructSize 88

.global PortEnableInterrupt
.global PortDisableInterrupt
.global PortRestoreThreadContextArch
.global vPortTimerHandler
.global vPortAPICSpuriousHandler

PortEnableInterrupt:
	sti
	ret

PortDisableInterrupt:
	cli
	ret
	
.align 8
vPortAPICSpuriousHandler:
	iretq

PortRestoreThreadContextArch:
	mov 1 * REGBYTES(%rdi),	%rbx
	mov 2 * REGBYTES(%rdi),	%rcx
	mov 3 * REGBYTES(%rdi),	%rdx
	mov 4 * REGBYTES(%rdi),	%rbp
	mov 6 * REGBYTES(%rdi),	%rsi

	mov %ss, %rax
	pushq %rax				  # ss
	pushq 7  * REGBYTES(%rdi) # rsp
	pushq 9  * REGBYTES(%rdi) # rflags
	pushq 8  * REGBYTES(%rdi) # cs
	pushq 10 * REGBYTES(%rdi) # rip

	# EOI
	mov $0xFEE000B0, %rax
	movl $0, (%rax)
	
	mov	0 * REGBYTES(%rdi),	%rax
	mov 5 * REGBYTES(%rdi),	%rdi
	iretq
	
.align 8
vPortTimerHandler:
	sub $InterruptStructSize, %rsp
	mov %rax, 0 * REGBYTES(%rsp)
	mov %rbx, 1 * REGBYTES(%rsp)
	mov %rcx, 2 * REGBYTES(%rsp)
	mov %rdx, 3 * REGBYTES(%rsp)
	mov %rbp, 4 * REGBYTES(%rsp)
	mov %rdi, 5 * REGBYTES(%rsp)
	mov %rsi, 6 * REGBYTES(%rsp)

	mov %rsp, %rax
	add $InterruptStructSize, %rax
	
	mov 0 * REGBYTES(%rax), %rbx
	mov %rbx, 10 * REGBYTES(%rsp) # rip before
	mov 1 * REGBYTES(%rax), %rbx
	mov %rbx, 8 * REGBYTES(%rsp) # cs
	mov 2 * REGBYTES(%rax), %rbx
	mov %rbx, 9 * REGBYTES(%rsp) # rflags
	mov 3 * REGBYTES(%rax), %rbx
	mov %rbx, 7 * REGBYTES(%rsp) # rsp before
	
	mov %rsp, %rdi # rdi = &icontext
	call Kernel_OnTimerHandler
	
	mov 1 * REGBYTES(%rsp),	%rbx
	mov 2 * REGBYTES(%rsp),	%rcx
	mov 3 * REGBYTES(%rsp),	%rdx
	mov 4 * REGBYTES(%rsp),	%rbp
	mov 5 * REGBYTES(%rsp),	%rdi
	mov 6 * REGBYTES(%rsp),	%rsi
	
	# EOI
	mov $0xFEE000B0, %rax
	movl $0, (%rax)

	mov 0 * REGBYTES(%rsp),	%rax
	add $InterruptStructSize, %rsp
	iretq
