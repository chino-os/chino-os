# Copyright (c) SunnyCase. All rights reserved.
# Licensed under the Apache license. See LICENSE file in the project root for full license information.

.global emulator_start_schedule
.global emulator_yield_context
.global emulator_dispatch_irq

.extern emulator_restore_irq
.extern ke_handle_irq

STACK_TOP_OFFSET    = 16
STACK_BOTTOM_OFFSET = 24

.text
.align 16

# rsp + 16 = entry_arg
# rsp + 8 = entry_point
# rsp + 0 = thread_main_thunk
# rcx = thread
emulator_start_schedule:
    mov     STACK_TOP_OFFSET(%rcx),     %rsp
    # set stack limit
    mov     %gs:0x30,   %rax     # TIB
    mov     STACK_TOP_OFFSET(%rcx),     %rdx     # StackBase
    mov     %rdx,       8(%rax)
    mov     STACK_BOTTOM_OFFSET(%rcx),  %rdx     # StackLimit
    mov     %rdx,       16(%rax)
    mov     $1,     %rcx
    call    emulator_restore_irq # enable irq
    pop     %rax
    pop     %rcx
    pop     %rdx
    jmp     *%rax
    
# rcx = old_thread
# rdx = new_thread
# r8 = scheduled
emulator_yield_context:
    # 1. save non-volatile regs
    lea     _emulator_yield_context_return(%rip),   %rax # save return address
    push    %rax
    push    %rbx
    push    %rbp
    push    %rsi
    push    %rdi
    push    %r12
    push    %r13
    push    %r14
    push    %r15
    push    $1                              # mark yield-style restore
    mov     %rsp,   STACK_TOP_OFFSET(%rcx)  # store stack_top

    # 2. restore
    mov     STACK_TOP_OFFSET(%rdx),   %rsp
    # 2.1. restore unscheduled
    test    %r8,    %r8
    jz      emulator_start_schedule

    # 2.2. restore scheduled
    pop     %rax            # pop mark
    test    %rax,   %rax
    jz      _emulator_non_yield_restore_context
    # 2.2.1. restore yield-like
    pop     %r15
    pop     %r14
    pop     %r13
    pop     %r12
    pop     %rdi
    pop     %rsi
    pop     %rbp
    pop     %rbx
    pop     %rax            # load return address
    jmp     *%rax
    
    # 2.2.2. restore non-yield-like
_emulator_non_yield_restore_context:
    jmp     .

_emulator_yield_context_return:
    ret
    
# rsp + 8 = old rip
# rsp + 0 = irq_number
emulator_dispatch_irq:
    # save volatile regs
    push    %rax
    push    %rcx
    push    %rdx
    push    %r8
    push    %r9
    push    %r10
    push    %r11

    mov     7*8(%rsp), %rcx
    call    ke_handle_irq

    # restore volatile regs
    pop     %r11
    pop     %r10
    pop     %r9
    pop     %r8
    pop     %rdx
    pop     %rcx
    pop     %rax

    addq    $8, %rsp
    ret
