
.global PortEnterKernel
.global PortHaltProcessor

PortEnterKernel:
	mov		0x0(%rdi), %rsp
	jmp		*%rsi

PortHaltProcessor:
	hlt