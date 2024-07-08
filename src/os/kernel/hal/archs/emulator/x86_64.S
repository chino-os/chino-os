# Copyright (c) SunnyCase. All rights reserved.
# Licensed under the Apache license. See LICENSE file in the project root for full license information.

.global emulator_start_schedule
.global emulator_restore_context
.global emulator_dispatch_irq

.extern emulator_restore_irq
.extern ke_handle_irq

.text
.align 16

# rsp + 16 = entry_arg
# rsp + 8 = entry_point
# rsp + 0 = thread_main_thunk
emulator_start_schedule:
    mov     $1,         %rcx
    call    emulator_restore_irq # enable irq
    pop     %rax
    pop     %rcx
    pop     %rdx
    jmp     *%rax

# rcx = context
emulator_restore_context:
    mov 8*1(%rcx), %rbx
    mov 8*3(%rcx), %rdx
    mov 8*4(%rcx), %rsp
    mov 8*5(%rcx), %rbp
    mov 8*6(%rcx), %rsi
    mov 8*7(%rcx), %rdi
    mov 8*8(%rcx), %r8
    mov 8*9(%rcx), %r9
    mov 8*10(%rcx), %r10
    mov 8*11(%rcx), %r11
    mov 8*12(%rcx), %r12
    mov 8*13(%rcx), %r13
    mov 8*14(%rcx), %r14
    mov 8*15(%rcx), %r15

    mov 8*16(%rcx), %rax    # rip
    mov %rax, -8(%rsp)
    mov 8*0(%rcx), %rax
    mov 8*2(%rcx), %rcx
    jmp *%rax
    
# rsp + 8 = old rip
# rsp + 0 = irq_number
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
    call    ke_handle_irq

    # pop volatile regs
    pop     %r11
    pop     %r10
    pop     %r9
    pop     %r8
    pop     %rdx
    pop     %rcx
    pop     %rax

    addq    $8, %rsp
    ret
