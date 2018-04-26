
.global PortEnterKernel

PortEnterKernel:
	mov		0x0(%rdi), %rsp
	jmp		*%rsi