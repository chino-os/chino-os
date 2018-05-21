#define REGBYTES 8
#define InterruptStructSize 88

.global ArchHaltProcessor
.global ArchEnableInterrupt
.global ArchDisableInterrupt
.global ArchRestoreThreadContextArch
.global ArchTimerHandler
.global ArchAPICSpuriousHandler

ArchHaltProcessor:
	wfi
	mov pc, lr

ArchEnableInterrupt:
	mov pc, lr

ArchDisableInterrupt:
	mov pc, lr
	
.align 4
ArchAPICSpuriousHandler:
	mov pc, lr

ArchRestoreThreadContextArch:
	mov pc, lr
	
.align 4
ArchTimerHandler:
	mov pc, lr
