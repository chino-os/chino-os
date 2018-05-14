
.global BSPEnterKernel
.global BSPHaltProcessor

BSPEnterKernel:
	mov		0x0(%rdi), %rsp
	jmp		*%rsi

BSPHaltProcessor:
	hlt