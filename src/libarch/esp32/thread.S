#define REGBYTES 4
#define MAXBasePriority 0xE0

.global ArchHaltProcessor
.global ArchEnableInterrupt
.global ArchDisableInterrupt
.global ArchRestoreThreadContextArch
.global PendSV_Handler

.section .text

.align 4
ArchHaltProcessor:
	ret
	
.align 4
ArchEnableInterrupt:
	ret
	
.align 4
ArchDisableInterrupt:
	ret
	
  .type  PendSV_Handler, %function
PendSV_Handler:
	ret
restore:
	ret
