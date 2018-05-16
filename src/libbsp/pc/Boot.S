.global BSPEnterKernel

BSPEnterKernel:
	mov		0x0(%rdi), %rsp
	jmp		*%rsi
