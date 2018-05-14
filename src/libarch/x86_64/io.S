
.global PortIOReadUInt8
.global PortIOWriteUInt8
.global PortIOReadUInt32String

PortIOReadUInt8:
	mov	%di,	%dx
	in	(%dx),	%al
	ret

PortIOWriteUInt8:
	mov	%di,	%dx
	mov %si,	%ax
	out	%al,	(%dx)
	ret

PortIOReadUInt32String:
	mov %rdx,	%rcx
	mov %di,	%dx
	mov %rsi,	%rdi
	rep insl (%dx), (%rdi)
	ret