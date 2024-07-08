# Copyright (c) SunnyCase. All rights reserved.
# Licensed under the Apache license. See LICENSE file in the project root for full license information.

.global emulator_restore_context
.global emulator_dispatch_irq

.extern ke_handle_irq

.text
.align 16

# rcx = context
emulator_restore_context:
    mov 8*3(%rcx), %rdx
    mov 8*4(%rcx), %rsp
    mov 8*8(%rcx), %r8
    mov 8*16(%rcx), %rax    # rip
    mov 8*2(%rcx), %rcx
    jmp *%rax
    
# rsp + 0 = irq_number
# rsp + 8 = old rip
emulator_dispatch_irq:
    # push volatile regs
    push    %rax
    push    %rcx
    push    %rdx
    push    %r8
    push    %r9
    push    %r10
    push    %r11

    mov     7*8(%rsp), %rcx
    push    $0
    call    ke_handle_irq

    # pop volatile regs
    pop     %r11
    pop     %r11
    pop     %r10
    pop     %r9
    pop     %r8
    pop     %rdx
    pop     %rcx
    pop     %rax

    addq    $16, %rsp
    jmp     *-8(%rsp)
