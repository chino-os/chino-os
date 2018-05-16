.global BSPKernelEntry

BSPKernelEntry:
	mov		0x0(%rdi), %rsp
	jmp		Kernel_Main
