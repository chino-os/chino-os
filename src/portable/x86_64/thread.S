#define REGBYTES 8

.global PortEnableInterrupt
.global PortDisableInterrupt

PortEnableInterrupt:
	sti
	ret

PortDisableInterrupt:
	cli
	ret
